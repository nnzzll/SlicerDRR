/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// DRRGenerator Logic includes
#include "vtkSlicerDRRGeneratorLogic.h"
#include <vtkSlicerVolumeRenderingLogic.h>

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLCPURayCastVolumeRenderingDisplayNode.h>
#include <vtkMRMLGPURayCastVolumeRenderingDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumePropertyNode.h>
#include <vtkMRMLVolumeRenderingDisplayNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageFlip.h>
#include <vtkImageLuminance.h>
#include <vtkIntArray.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkWindowToImageFilter.h>

// STD includes
#include <cassert>
#include <iostream>

// Slicer includes
#include <qMRMLThreeDView.h>

vtkStandardNewMacro(vtkSlicerDRRGeneratorLogic);

vtkSlicerDRRGeneratorLogic::vtkSlicerDRRGeneratorLogic() {}

vtkSlicerDRRGeneratorLogic::~vtkSlicerDRRGeneratorLogic() {}

void vtkSlicerDRRGeneratorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

void vtkSlicerDRRGeneratorLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

void vtkSlicerDRRGeneratorLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

void vtkSlicerDRRGeneratorLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

void vtkSlicerDRRGeneratorLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node)) {}

void vtkSlicerDRRGeneratorLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node)) {}

vtkMRMLVolumeRenderingDisplayNode* vtkSlicerDRRGeneratorLogic::createVolumeRenderingNode(
    vtkMRMLScalarVolumeNode* volumeNode)
{
  // Create Volume Rendering Display Node
  std::string nodeName = std::string(volumeNode->GetName());
  auto displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      this->getNodeByName<vtkMRMLGPURayCastVolumeRenderingDisplayNode>(nodeName + "DisplayNode", true));
  volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());

  // Create Volume Property Node
  auto vpn = this->getNodeByName<vtkMRMLVolumePropertyNode>("DRRVolumeProperty", true);
  this->VolumePropertyNode = vpn;
  this->SetupVolumePropertyNode(vpn);
  displayNode->SetAndObserveVolumePropertyNodeID(vpn->GetID());
  return displayNode;
}

void vtkSlicerDRRGeneratorLogic::SetupVolumePropertyNode(vtkMRMLVolumePropertyNode* vpn)
{
  vtkNew<vtkPiecewiseFunction> gradientOpacity, scalarOpacity;
  vtkNew<vtkColorTransferFunction> color;

  // 默认的CT-X-ray体绘制参数, 详见Modules/Loadable/VolumeRendering/Resources/presets.xml
  gradientOpacity->AddPoint(0, 1);
  gradientOpacity->AddPoint(255, 1);
  scalarOpacity->AddPoint(-3024, 0);
  scalarOpacity->AddPoint(-200, 0);
  scalarOpacity->AddPoint(1500, 0.05);
  scalarOpacity->AddPoint(3071, 0.05);
  color->AddRGBPoint(-3024, 1, 1, 1);
  color->AddRGBPoint(3071, 1, 1, 1);
  vpn->SetGradientOpacity(gradientOpacity);
  vpn->SetScalarOpacity(scalarOpacity);
  vpn->SetColor(color);

  vtkVolumeProperty* vp = vpn->GetVolumeProperty();
  vp->ShadeOff();
  vp->SetSpecular(0.2);
  vp->SetAmbient(0.1);
  vp->SetDiffuse(0.9);
  vp->SetSpecularPower(10);
  vp->SetInterpolationTypeToLinear();
}

void vtkSlicerDRRGeneratorLogic::SetThreeDView(qMRMLThreeDView* threeDView)
{
  this->view = threeDView;
  this->camera = threeDView->cameraNode();
  auto cameraNode = view->cameraNode();
  auto transformNode = this->getNodeByName<vtkMRMLLinearTransformNode>("CameraTransform", true);
  cameraNode->SetAndObserveTransformNodeID(transformNode->GetID());
  threeDView->renderWindow()->Render();
}

void vtkSlicerDRRGeneratorLogic::ConvertEigenToVTK(Eigen::Matrix4d in, vtkMatrix4x4* out)
{
  // transpose Eigen Matrix due to difference storage order
  in.transposeInPlace();
  double* inPointer = in.data();
  double* outPointer = out->GetData();
  for (int i = 0; i < 16; i++)
  {
    outPointer[i] = inPointer[i];
  }
}

void vtkSlicerDRRGeneratorLogic::Rx(double isocenter[3], double angle, Eigen::Matrix4d& out)
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

void vtkSlicerDRRGeneratorLogic::Ry(double isocenter[3], double angle, Eigen::Matrix4d& out)
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

void vtkSlicerDRRGeneratorLogic::Rz(double isocenter[3], double angle, Eigen::Matrix4d& out)
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

void vtkSlicerDRRGeneratorLogic::resetRotation()
{
  this->currentVolumeRot = Eigen::Matrix4d::Identity();
  this->oldRotation[0] = 0;
  this->oldRotation[1] = 0;
  this->oldRotation[2] = 0;
}

void vtkSlicerDRRGeneratorLogic::updateVolumePropertyNode(double wl, double ww, double op)
{
  if (!this->VolumePropertyNode) return;

  auto scalarOpacity = VolumePropertyNode->GetScalarOpacity();
  scalarOpacity->RemoveAllPoints();
  scalarOpacity->AddPoint(-3024, 0);
  scalarOpacity->AddPoint(wl - 0.5 * ww, 0);
  scalarOpacity->AddPoint(wl + 0.5 * ww, op);
  scalarOpacity->AddPoint(3071, op);
}

void vtkSlicerDRRGeneratorLogic::applyDRR(vtkMRMLScalarVolumeNode* volumeNode, vtkMRMLScalarVolumeNode* drrNode,
                                          double angle, double rotation[3], double translation[3], int size[3])
{
  this->view->setFixedSize(size[0], size[1]);
  double bounds[6];
  volumeNode->GetBounds(bounds);
  double center[3] = {
      0.5 * (bounds[0] + bounds[1]),
      0.5 * (bounds[2] + bounds[3]),
      0.5 * (bounds[4] + bounds[5]),
  };

  // 计算CT的旋转
  Eigen::Vector4d translationVec;
  const double dtr = 0.017453292519943295;
  rotation[0] *= dtr;
  rotation[1] *= dtr;
  rotation[2] *= dtr;
  // ! 每一次旋转都是在当前的基础上沿自身的某个轴转动
  // ! 因此可以避免万向锁的情况
  double deltaRx = rotation[0] - oldRotation[0];
  double deltaRy = rotation[1] - oldRotation[1];
  double deltaRz = rotation[2] - oldRotation[2];
  memcpy(oldRotation, rotation, 3 * sizeof(double));
  Eigen::Matrix4d deltaR;
  if (std::abs(deltaRx) > 1e-8)
  {
    this->Rx(center, deltaRx, deltaR);
  }
  else if (std::abs(deltaRy) > 1e-8)
  {
    this->Ry(center, deltaRy, deltaR);
  }
  else
  {
    this->Rz(center, deltaRz, deltaR);
  }
  Eigen::Matrix4d volumeRot = this->currentVolumeRot * deltaR;
  this->currentVolumeRot = volumeRot;
  translationVec << translation[0], translation[1], translation[2], 0;
  volumeRot.col(3) += translationVec;

  // 设置体数据的旋转
  vtkNew<vtkMatrix4x4> transformMatrix;
  auto transformNode = this->getNodeByName<vtkMRMLLinearTransformNode>("VolumeTransform");
  this->ConvertEigenToVTK(volumeRot, transformMatrix);
  transformNode->SetMatrixTransformToParent(transformMatrix);

  // 设置相机的旋转
  angle *= dtr;
  Eigen::Matrix4d cameraRot, rx, ry, rz;
  Rx(center, 0, rx);
  Ry(center, 0, ry);
  Rz(center, -angle, rz);
  cameraRot = rz * ry * rx;
  vtkNew<vtkMatrix4x4> camRotMatrix;
  this->ConvertEigenToVTK(cameraRot, camRotMatrix);
  auto cameraTransformNode = this->getNodeByName<vtkMRMLLinearTransformNode>("CameraTransform");
  cameraTransformNode->SetMatrixTransformToParent(camRotMatrix);

  this->camera->ResetClippingRange();  // 更新渲染的显示范围

  this->getDRRFromVolumeRendering(drrNode);
}

void vtkSlicerDRRGeneratorLogic::getDRRFromVolumeRendering(vtkMRMLScalarVolumeNode* drrNode)
{
  // 将体绘制转换成图像
  auto rw = this->view->renderWindow();
  rw->Render();  // 更新渲染窗口的内容
  vtkNew<vtkWindowToImageFilter> wti;
  wti->SetInput(rw);
  wti->Update();
  vtkNew<vtkImageFlip> vflip;
  vflip->SetInputData(wti->GetOutput());
  vflip->SetFilteredAxis(1);
  vflip->Update();

  // 将RGB图像转换成灰度图
  vtkNew<vtkImageExtractComponents> extract;
  extract->SetInputData(vflip->GetOutput());
  extract->SetComponents(0, 1, 2);
  vtkNew<vtkImageLuminance> luminance;
  luminance->SetInputConnection(extract->GetOutputPort());
  luminance->Update();
  drrNode->SetAndObserveImageData(luminance->GetOutput());
}

void vtkSlicerDRRGeneratorLogic::getFiducialPosition(vtkMRMLMarkupsFiducialNode* pointNode, IJKVec& ijkPoints)
{
  double worldPosition[4]{0, 0, 0, 1}, imagePoint[3]{0, 0, 1};
  ijkPoints.clear();
  auto renderer = this->view->renderWindow()->GetRenderers()->GetFirstRenderer();
  for (int i = 0; i < pointNode->GetNumberOfControlPoints(); i++)
  {
    pointNode->GetNthControlPointPositionWorld(i, worldPosition);
    renderer->SetWorldPoint(worldPosition);
    renderer->WorldToDisplay();
    renderer->GetDisplayPoint(imagePoint);
    imagePoint[1] = this->view->size().height() - imagePoint[1];
    ijkPoints.push_back({imagePoint[0], imagePoint[1]});
  }
}

void vtkSlicerDRRGeneratorLogic::initializeCamera(vtkMRMLScalarVolumeNode* volumeNode)
{
  double bounds[6];
  volumeNode->GetBounds(bounds);
  double center[3] = {
      0.5 * (bounds[0] + bounds[1]),
      0.5 * (bounds[2] + bounds[3]),
      0.5 * (bounds[4] + bounds[5]),
  };

  // 先将相机设置为初始位置
  double position[3], focalPoint[3], viewUp[3]{0, 0, 1};
  memcpy(focalPoint, center, 3 * sizeof(double));
  memcpy(position, center, 3 * sizeof(double));
  position[1] += 1000;  // 焦距默认设置为1000
  this->camera->SetPosition(position);
  this->camera->SetFocalPoint(focalPoint);
  this->camera->SetViewUp(viewUp);
}