#pragma once

#include "DRRGeneratorMacro.h"
#include <itkeigen/Eigen/Core>
#include <vtkSmartPointer.h>
#include <vtkTimeStamp.h>

class vtkImageData;
class DRRGenerator
{
 private:
  DRRGenerator(const DRRGenerator&) = delete;
  void operator=(const DRRGenerator&) = delete;

  void ComputeTransform();
  void Initialize();
  short Evaluate(Eigen::Vector4d& point);
  void Rx(double isocenter[3], double angle, Eigen::Matrix4d& out);
  void Ry(double isocenter[3], double angle, Eigen::Matrix4d& out);
  void Rz(double isocenter[3], double angle, Eigen::Matrix4d& out);
  void ThreadedRequestData(int imin, int imax, int jmin, int jmax);

  void ImageToCamera(int i, int j, Eigen::Vector4d& camPos);
  void ImageToCamera(int i, int j, double camPos[3]);
  void CameraToImage(const Eigen::Vector4d& camPos, double& i, double& j);
  void CameraToImage(const double camPos[3], double imgPos[2]);

  void Modified();

  double m_Angle;                     // 相机绕病人Z轴旋转的角度(弧度)
  double m_SourceToDetectorDistance;  // 相机到成像平面距离
  double m_Threshold;                 // 忽略低于该阈值的Voxel
  double m_Rotation[3];               // Volume绕isocenter旋转的角度(弧度)
  double m_Translation[3];            // Volume相对isocenter平移的距离(mm)
  double m_Isocenter[3];              // 旋转中心, 默认为CT体数据的中心点
  double m_Origin[3];                 // DRR图像的origin, 由DRR的size,spacing, scd计算得到
  double m_Spacing[3];                // DRR图像的spacing
  int m_Size[3];                      // DRR图像的size
  int m_VolumeSize[3];                // CT图像的Size
  double m_VolumeSpacing[3];          // CT图像的Spacing
  int m_BlockSize;                    //  每个线程计算blockSize*blockSize大小的区域
  int row, col;                       // 共有row * col 个block
  double sourceWorld[3];              // 相机原点在LPS下的坐标
  short* volumePointer;               // CT体数据的数据指针
  short* imagePointer;                // DRR图像的数据指针
  size_t volumeLength;                // CT体素的个数
  Eigen::Matrix4d m_Transform;        // 相机坐标到LPS坐标的转换矩阵
  vtkSmartPointer<vtkImageData> m_DRR;
  vtkTimeStamp updateTime;
  vtkTimeStamp modifyTime;

 public:
  DRRGenerator();
  ~DRRGenerator() = default;

  VelSetMacro(Angle, double);
  VelGetMacro(Angle, double);

  VelSetMacro(SourceToDetectorDistance, double);
  VelGetMacro(SourceToDetectorDistance, double);

  VelSetMacro(Threshold, double);
  VelGetMacro(Threshold, double);

  VelSetMacro(BlockSize, int);
  VelGetMacro(BlockSize, int);

  VelSetVector3Macro(Isocenter, double);
  VelGetVector3Macro(Isocenter, double);

  VelSetVector3Macro(Origin, double);
  VelGetVector3Macro(Origin, double);

  VelSetVector3Macro(Size, int);
  VelGetVector3Macro(Size, int);

  VelSetVector3Macro(Spacing, double);
  VelGetVector3Macro(Spacing, double);

  VelSetVector3Macro(Rotation, double);
  VelGetVector3Macro(Rotation, double);

  VelSetVector3Macro(Translation, double);
  VelGetVector3Macro(Translation, double);

  VelGetMacro(Transform, Eigen::Matrix4d);

  void SetInputData(vtkImageData* image, double spacing[3] = nullptr);
  vtkSmartPointer<vtkImageData> GetOutput();
  void GetFiducialPosition(double point3D[3], double point2D[2]);

  void Update();
};