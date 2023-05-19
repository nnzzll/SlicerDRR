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
#include "vtkMRMLCPURayCastVolumeRenderingDisplayNode.h"
#include "vtkMRMLGPURayCastVolumeRenderingDisplayNode.h"
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumePropertyNode.h>
#include <vtkMRMLVolumeRenderingDisplayNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>

// STD includes
#include <cassert>

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