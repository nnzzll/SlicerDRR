#include "DRRGenerator.h"

#include <iostream>
#include <thread>
#include <vector>

#include <itkeigen/Eigen/LU>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageFlip.h>
#include <vtkNew.h>
#include <vtkPlane.h>

DRRGenerator::DRRGenerator()
{
  this->SetAngle(0);
  this->SetSourceToDetectorDistance(1000);
  this->SetThreshold(0);
  this->SetBlockSize(32);
  double rot[3]{}, trans[3]{}, sp[3]{1., 1., 1.};
  int sz[3]{256, 256, 1};
  this->SetRotation(rot);
  this->SetTranslation(trans);
  this->SetSpacing(sp);
  this->SetSize(sz);
}

void DRRGenerator::Initialize()
{
  m_Origin[0] = -m_Spacing[0] * static_cast<double>(m_Size[0] - 1) * 0.5;
  m_Origin[1] = -m_Spacing[1] * static_cast<double>(m_Size[1] - 1) * 0.5;
  m_Origin[2] = -m_SourceToDetectorDistance;

  m_DRR = vtkSmartPointer<vtkImageData>::New();
  m_DRR->SetDimensions(m_Size);
  m_DRR->AllocateScalars(VTK_SHORT, 1);
  m_DRR->SetSpacing(1.0, 1.0, 1.0);  // ! 在MRMLNode中记录Spacing,故此处设为0
  imagePointer = static_cast<short*>(m_DRR->GetScalarPointer());

  row = m_Size[0] / m_BlockSize;
  col = m_Size[1] / m_BlockSize;
  if (m_Size[0] % m_BlockSize != 0 || m_Size[1] % m_BlockSize != 0)
  {
    std::cerr << "DRR Size "
              << "(" << m_Size[0] << ", " << m_Size[1] << ")"
              << " Cannot be Devided by BlockSize"
              << "(" << m_BlockSize << ")" << std::endl;
  }
}

void DRRGenerator::Modified()
{
  this->modifyTime.Modified();
}

void DRRGenerator::Rx(double isocenter[3], double angle, Eigen::Matrix4d& out)
{
  double y = isocenter[1], z = isocenter[2];
  // clang-format off
  out <<
    1, 0, 0, 0,
    0, cos(angle), -sin(angle), y * (1-cos(angle)) + z * sin(angle),
    0, sin(angle),  cos(angle), z * (1-cos(angle)) - y * sin(angle),
    0, 0, 0, 1;
  // clang-format on
}

void DRRGenerator::Ry(double isocenter[3], double angle, Eigen::Matrix4d& out)
{
  double x = isocenter[0], z = isocenter[2];
  // clang-format off
  out <<
     cos(angle), 0, sin(angle),  x * (1-cos(angle)) - z * sin(angle),
     0, 1, 0, 0,
    -sin(angle), 0, cos(angle),  z * (1-cos(angle)) + x * sin(angle),
     0, 0, 0, 1;
  // clang-format on
}

void DRRGenerator::Rz(double isocenter[3], double angle, Eigen::Matrix4d& out)
{
  double y = isocenter[1], x = isocenter[0];
  // clang-format off
  out <<
    cos(angle), -sin(angle), 0, x * (1-cos(angle)) + y * sin(angle),
    sin(angle),  cos(angle), 0, y * (1-cos(angle)) - x * sin(angle),
    0, 0, 1, 0, 
    0, 0, 0, 1;
  // clang-format on
}

short DRRGenerator::Evaluate(Eigen::Vector4d& point)
{
  short pixval;
  int cIndex[3];

  float firstIntersection[3];
  float alphaX1, alphaXN, alphaXmin, alphaXmax;
  float alphaY1, alphaYN, alphaYmin, alphaYmax;
  float alphaZ1, alphaZN, alphaZmin, alphaZmax;
  float alphaMin, alphaMax;
  float alphaX, alphaY, alphaZ, alphaCmin, alphaCminPrev;
  float alphaUx, alphaUy, alphaUz;
  float alphaIntersectionUp[3], alphaIntersectionDown[3];
  float d12, value;
  float firstIntersectionIndex[3];
  int firstIntersectionIndexUp[3], firstIntersectionIndexDown[3];
  int iU, jU, kU;

  const short minOutputValue = VTK_SHORT_MIN;
  const short maxOutputValue = VTK_SHORT_MAX;

  Eigen::Vector4d drrWorld;
  drrWorld = m_Transform * point;
  drrWorld /= drrWorld(3);

  float rayVector[3];
  rayVector[0] = static_cast<float>(drrWorld(0) - sourceWorld[0]);
  rayVector[1] = static_cast<float>(drrWorld(1) - sourceWorld[1]);
  rayVector[2] = static_cast<float>(drrWorld(2) - sourceWorld[2]);

  /* Calculate the parametric  values of the first  and  the  last
  intersection points of  the  ray  with the X,  Y, and Z-planes  that
  define  the  CT volume. */
  if (rayVector[0] != 0)
  {
    alphaX1 = (0.0 - sourceWorld[0]) / rayVector[0];
    alphaXN = (m_VolumeSize[0] * m_VolumeSpacing[0] - sourceWorld[0]) / rayVector[0];
    alphaXmin = std::min(alphaX1, alphaXN);
    alphaXmax = std::max(alphaX1, alphaXN);
  }
  else
  {
    alphaXmin = -2;
    alphaXmax = 2;
  }

  if (rayVector[1] != 0)
  {
    alphaY1 = (0.0 - sourceWorld[1]) / rayVector[1];
    alphaYN = (m_VolumeSize[1] * m_VolumeSpacing[1] - sourceWorld[1]) / rayVector[1];
    alphaYmin = std::min(alphaY1, alphaYN);
    alphaYmax = std::max(alphaY1, alphaYN);
  }
  else
  {
    alphaYmin = -2;
    alphaYmax = 2;
  }

  if (rayVector[2] != 0)
  {
    alphaZ1 = (0.0 - sourceWorld[2]) / rayVector[2];
    alphaZN = (m_VolumeSize[2] * m_VolumeSpacing[2] - sourceWorld[2]) / rayVector[2];
    alphaZmin = std::min(alphaZ1, alphaZN);
    alphaZmax = std::max(alphaZ1, alphaZN);
  }
  else
  {
    alphaZmin = -2;
    alphaZmax = 2;
  }

  /* Get the very first and the last alpha values when the ray
  intersects with the CT volume. */
  alphaMin = std::max(std::max(alphaXmin, alphaYmin), alphaZmin);
  alphaMax = std::min(std::min(alphaXmax, alphaYmax), alphaZmax);

  /* Calculate the parametric values of the first intersection point
  of the ray with the X, Y, and Z-planes after the ray entered the
  CT volume. */
  firstIntersection[0] = sourceWorld[0] + alphaMin * rayVector[0];
  firstIntersection[1] = sourceWorld[1] + alphaMin * rayVector[1];
  firstIntersection[2] = sourceWorld[2] + alphaMin * rayVector[2];

  /* Transform world coordinate to the continuous index of the CT volume*/
  firstIntersectionIndex[0] = firstIntersection[0] / m_VolumeSpacing[0];
  firstIntersectionIndex[1] = firstIntersection[1] / m_VolumeSpacing[1];
  firstIntersectionIndex[2] = firstIntersection[2] / m_VolumeSpacing[2];

  firstIntersectionIndexUp[0] = (int)ceil(firstIntersectionIndex[0]);
  firstIntersectionIndexUp[1] = (int)ceil(firstIntersectionIndex[1]);
  firstIntersectionIndexUp[2] = (int)ceil(firstIntersectionIndex[2]);

  firstIntersectionIndexDown[0] = (int)floor(firstIntersectionIndex[0]);
  firstIntersectionIndexDown[1] = (int)floor(firstIntersectionIndex[1]);
  firstIntersectionIndexDown[2] = (int)floor(firstIntersectionIndex[2]);

  if (rayVector[0] == 0)
  {
    alphaX = 2;
  }
  else
  {
    alphaIntersectionUp[0] =
        (firstIntersectionIndexUp[0] * m_VolumeSpacing[0] - sourceWorld[0]) / rayVector[0];
    alphaIntersectionDown[0] =
        (firstIntersectionIndexDown[0] * m_VolumeSpacing[0] - sourceWorld[0]) / rayVector[0];
    alphaX = std::max(alphaIntersectionUp[0], alphaIntersectionDown[0]);
  }

  if (rayVector[1] == 0)
  {
    alphaY = 2;
  }
  else
  {
    alphaIntersectionUp[1] =
        (firstIntersectionIndexUp[1] * m_VolumeSpacing[1] - sourceWorld[1]) / rayVector[1];
    alphaIntersectionDown[1] =
        (firstIntersectionIndexDown[1] * m_VolumeSpacing[1] - sourceWorld[1]) / rayVector[1];
    alphaY = std::max(alphaIntersectionUp[1], alphaIntersectionDown[1]);
  }

  if (rayVector[2] == 0)
  {
    alphaZ = 2;
  }
  else
  {
    alphaIntersectionUp[2] =
        (firstIntersectionIndexUp[2] * m_VolumeSpacing[2] - sourceWorld[2]) / rayVector[2];
    alphaIntersectionDown[2] =
        (firstIntersectionIndexDown[2] * m_VolumeSpacing[2] - sourceWorld[2]) / rayVector[2];
    alphaZ = std::max(alphaIntersectionUp[2], alphaIntersectionDown[2]);
  }

  /* Calculate alpha incremental values when the ray intercepts with x, y, and z-planes */
  if (rayVector[0] != 0)
  {
    alphaUx = m_VolumeSpacing[0] / std::abs(rayVector[0]);
  }
  else
  {
    alphaUx = 999;
  }
  if (rayVector[1] != 0)
  {
    alphaUy = m_VolumeSpacing[1] / std::abs(rayVector[1]);
  }
  else
  {
    alphaUy = 999;
  }
  if (rayVector[2] != 0)
  {
    alphaUz = m_VolumeSpacing[2] / std::abs(rayVector[2]);
  }
  else
  {
    alphaUz = 999;
  }

  /* Calculate voxel index incremental values along the ray path. */
  if (sourceWorld[0] < drrWorld(0))
  {
    iU = 1;
  }
  else
  {
    iU = -1;
  }
  if (sourceWorld[1] < drrWorld(1))
  {
    jU = 1;
  }
  else
  {
    jU = -1;
  }

  if (sourceWorld[2] < drrWorld(2))
  {
    kU = 1;
  }
  else
  {
    kU = -1;
  }

  d12 = 0.0; /* Initialize the sum of the voxel intensities along the ray path to zero. */

  /* Initialize the current ray position. */
  alphaCmin = std::min(std::min(alphaX, alphaY), alphaZ);

  /* Initialize the current voxel index. */
  cIndex[0] = firstIntersectionIndexDown[0];
  cIndex[1] = firstIntersectionIndexDown[1];
  cIndex[2] = firstIntersectionIndexDown[2];

  while (alphaCmin < alphaMax) /* Check if the ray is still in the CT volume */
  {
    /* Store the current ray position */
    alphaCminPrev = alphaCmin;

    if ((alphaX <= alphaY) && (alphaX <= alphaZ))
    {
      /* Current ray front intercepts with x-plane. Update alphaX. */
      alphaCmin = alphaX;
      cIndex[0] = cIndex[0] + iU;
      alphaX = alphaX + alphaUx;
    }
    else if ((alphaY <= alphaX) && (alphaY <= alphaZ))
    {
      /* Current ray front intercepts with y-plane. Update alphaY. */
      alphaCmin = alphaY;
      cIndex[1] = cIndex[1] + jU;
      alphaY = alphaY + alphaUy;
    }
    else
    {
      /* Current ray front intercepts with z-plane. Update alphaZ. */
      alphaCmin = alphaZ;
      cIndex[2] = cIndex[2] + kU;
      alphaZ = alphaZ + alphaUz;
    }

    /* If it is a valid index, get the voxel intensity. */
    if (cIndex[0] >= 0 && cIndex[1] >= 0 && cIndex[2] >= 0 && cIndex[0] < m_VolumeSize[0] &&
        cIndex[1] < m_VolumeSize[1] && cIndex[2] < m_VolumeSize[2])
    {
      size_t index =
          cIndex[0] + cIndex[1] * m_VolumeSize[0] + cIndex[2] * m_VolumeSize[1] * m_VolumeSize[0];
      value = static_cast<float>(volumePointer[index]);
      if (value > m_Threshold) /* Ignore voxels whose intensities are below the threshold. */
      {
        d12 += (alphaCmin - alphaCminPrev) * (value - m_Threshold);
      }
    }
  }

  pixval = d12 < minOutputValue   ? minOutputValue
           : d12 > maxOutputValue ? maxOutputValue
                                  : static_cast<short>(d12);

  return pixval;
}

void DRRGenerator::ComputeTransform()
{
  Eigen::Matrix4d rx, ry, rz;
  Eigen::Vector4d translation;
  // ! 每一次旋转都是在当前的基础上沿自身的某个轴转动
  // ! 因此可以避免万向锁的情况
  double deltaRx = m_Rotation[0] - oldRotation[0];
  double deltaRy = m_Rotation[1] - oldRotation[1];
  double deltaRz = m_Rotation[2] - oldRotation[2];
  memcpy(oldRotation, m_Rotation, 3 * sizeof(double));
  Eigen::Matrix4d deltaR;
  if (std::abs(deltaRx) > 1e-8)
  {
    Rx(m_Isocenter, deltaRx, deltaR);
  }
  else if (std::abs(deltaRy) > 1e-8)
  {
    Ry(m_Isocenter, deltaRy, deltaR);
  }
  else
  {
    Rz(m_Isocenter, deltaRz, deltaR);
  }
  Eigen::Matrix4d volumeRot;
  volumeRot = currentVolumeRot * deltaR;
  currentVolumeRot = volumeRot;
  translation << m_Translation[0], m_Translation[1], m_Translation[2], 0;
  volumeRot.col(3) += translation;

  Eigen::Matrix4d gantryRot;
  Rx(m_Isocenter, 0, rx);
  Ry(m_Isocenter, 0, ry);
  Rz(m_Isocenter, -m_Angle, rz);
  gantryRot = rz * ry * rx;

  Eigen::Matrix4d cameraShift = Eigen::Matrix4d::Identity();
  translation << -m_Isocenter[0], m_SourceToDetectorDistance - m_Isocenter[1], -m_Isocenter[2], 0;
  cameraShift.col(3) += translation;

  Eigen::Matrix4d cameraRot;
  cameraRot << 1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1;

  // Camera2LPS
  m_Transform = (cameraRot * cameraShift * gantryRot * volumeRot).inverse();
  Eigen::Vector4d source = Eigen::Vector4d::Zero();
  source(3) = 1;
  Eigen::Vector4d sourceWorldVec;
  sourceWorldVec = m_Transform * source;
  sourceWorldVec /= sourceWorldVec(3);

  sourceWorld[0] = sourceWorldVec(0);
  sourceWorld[1] = sourceWorldVec(1);
  sourceWorld[2] = sourceWorldVec(2);
}

void DRRGenerator::ImageToCamera(int i, int j, Eigen::Vector4d& camPos)
{
  camPos << m_Origin[0] + i * m_Spacing[0], m_Origin[1] + j * m_Spacing[1], m_Origin[2], 1;
}

void DRRGenerator::ImageToCamera(int i, int j, double camPos[3])
{
  camPos[0] = m_Origin[0] + i * m_Spacing[0];
  camPos[1] = m_Origin[1] + j * m_Spacing[1];
  camPos[2] = m_Origin[2];
}

void DRRGenerator::CameraToImage(const Eigen::Vector4d& camPos, double& i, double& j)
{
  i = (camPos(0) - m_Origin[0]) / m_Spacing[0];
  j = (camPos(1) - m_Origin[1]) / m_Spacing[1];
}

void DRRGenerator::CameraToImage(const double camPos[3], double imgPos[2])
{
  imgPos[0] = (camPos[0] - m_Origin[0]) / m_Spacing[0];
  imgPos[1] = (camPos[1] - m_Origin[1]) / m_Spacing[1];
}

void DRRGenerator::ThreadedRequestData(int imin, int imax, int jmin, int jmax)
{
  Eigen::Vector4d point;
  for (int j = jmin; j < jmax; j++)
    for (int i = imin; i < imax; i++)
    {
      this->ImageToCamera(i, j, point);
      imagePointer[i + j * m_Size[0]] = this->Evaluate(point);
    }
}

void DRRGenerator::SetInputData(vtkImageData* image, double spacing[3])
{
  volumePointer = static_cast<short*>(image->GetScalarPointer());
  image->GetDimensions(m_VolumeSize);
  if (!spacing)
  {
    spacing[0] = spacing[1] = spacing[2] = 1.0;
  }
  memcpy(m_VolumeSpacing, spacing, 3 * sizeof(double));
  m_Isocenter[0] = m_VolumeSpacing[0] * static_cast<double>(m_VolumeSize[0]) / 2.0;
  m_Isocenter[1] = m_VolumeSpacing[1] * static_cast<double>(m_VolumeSize[1]) / 2.0;
  m_Isocenter[2] = m_VolumeSpacing[2] * static_cast<double>(m_VolumeSize[2]) / 2.0;
}

void DRRGenerator::GetFiducialPosition(double point3D[3], double point2D[2])
{
  if (modifyTime.GetMTime() > updateTime.GetMTime())
  {
    this->ComputeTransform();
    updateTime.Modified();
  }
  Eigen::Matrix4d ctlps2camera = this->m_Transform.inverse();
  Eigen::Vector4d point3DVec = Eigen::Vector4d::Ones();
  point3DVec(0) = point3D[0];
  point3DVec(1) = point3D[1];
  point3DVec(2) = point3D[2];
  Eigen::Vector4d point3DCamVec = ctlps2camera * point3DVec;
  point3DCamVec /= point3DCamVec(3);

  double origin[3], normal[3]{0, 0, 1};
  this->ImageToCamera(this->m_Size[0] / 2.0, this->m_Size[1] / 2.0, origin);
  double p1[3], p2[3]{0, 0, 0};
  p1[0] = point3DCamVec(0);
  p1[1] = point3DCamVec(1);
  p1[2] = point3DCamVec(2);

  double fiducialCam[3];
  double t = 0;
  vtkPlane::IntersectWithLine(p1, p2, normal, origin, t, fiducialCam);
  this->CameraToImage(fiducialCam, point2D);
  // ! 此时的DRR影像在Y轴是上下颠倒的,因此point2D的Y坐标需要取反
  point2D[1] = this->m_Size[1] - point2D[1];
}

void DRRGenerator::Update()
{
  // 第一次计算时, 设置一些初始参数, 避免重复计算
  if (modifyTime.GetMTime() > updateTime.GetMTime())
  {
    Initialize();
    this->ComputeTransform();
    updateTime.Modified();
  }

  std::vector<std::thread> pool;
  int imin, imax, jmin, jmax;
  for (int i = 0; i < row; i++)
    for (int j = 0; j < col; j++)
    {
      imin = i * m_BlockSize;
      imax = (i + 1) * m_BlockSize;
      jmin = j * m_BlockSize;
      jmax = (j + 1) * m_BlockSize;
      pool.push_back(std::thread(&DRRGenerator::ThreadedRequestData, this, imin, imax, jmin, jmax));
    }

  for (int i = 0; i < row * col; i++)
  {
    pool[i].join();
  }
}

vtkSmartPointer<vtkImageData> DRRGenerator::GetOutput()
{
  double range[2];
  this->m_DRR->GetScalarRange(range);
  size_t drrLength = this->m_Size[0] * this->m_Size[1] * this->m_Size[2];
  vtkSmartPointer<vtkImageData> outputImage = vtkSmartPointer<vtkImageData>::New();
  outputImage->SetDimensions(this->m_Size);
  outputImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  outputImage->SetSpacing(1.0, 1.0, 1.0);
  uint8_t* outputPtr = static_cast<uint8_t*>(outputImage->GetScalarPointer());
  for (size_t i = 0; i < drrLength; i++)
  {
    outputPtr[i] = static_cast<uint8_t>(255.0 * (static_cast<double>(imagePointer[i]) - range[0]) /
                                        (range[1] - range[0]));
  }

  vtkNew<vtkImageFlip> flipFilter;
  flipFilter->SetInputData(outputImage);
  flipFilter->SetFilteredAxes(1);
  flipFilter->Update();
  return flipFilter->GetOutput();
}

void DRRGenerator::Reset()
{
  this->currentVolumeRot = Eigen::Matrix4d::Identity();
  this->oldRotation[0] = 0;
  this->oldRotation[1] = 0;
  this->oldRotation[2] = 0;
}