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
#include "DRRGenerator.h"

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>
#include <chrono>

vtkStandardNewMacro(vtkSlicerDRRGeneratorLogic);

vtkSlicerDRRGeneratorLogic::vtkSlicerDRRGeneratorLogic()
{
  this->drrGen = std::make_shared<DRRGenerator>();
}

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

void vtkSlicerDRRGeneratorLogic::applyDRR(vtkMRMLScalarVolumeNode* ctVolume,
                                          vtkMRMLScalarVolumeNode* drrVolume, double angle,
                                          double threshold, double scd, double rotation[3],
                                          double translation[3], int size[3], double spacing[3])
{
  auto begin = std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count();

  const double dtr = 0.017453292519943295;
  double ctSpacing[3];
  ctVolume->GetSpacing(ctSpacing);
  rotation[0] *= dtr;
  rotation[1] *= dtr;
  rotation[2] *= dtr;
  angle *= dtr;

  this->drrGen->SetAngle(angle);
  this->drrGen->SetRotation(rotation);
  this->drrGen->SetTranslation(translation);
  this->drrGen->SetSourceToDetectorDistance(scd);
  this->drrGen->SetThreshold(threshold);
  this->drrGen->SetSpacing(spacing);
  this->drrGen->SetSize(size);
  this->drrGen->SetInputData(ctVolume->GetImageData(), ctSpacing);
  this->drrGen->Update();
  vtkSmartPointer<vtkImageData> drrImage = this->drrGen->GetOutput();
  drrVolume->SetAndObserveImageData(drrImage.GetPointer());
  drrVolume->StorableModified();
  drrVolume->Modified();
  auto end = std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();
  std::cout << "Time Used:" << end - begin << "ms" << std::endl;
}