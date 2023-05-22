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

// .NAME vtkSlicerDRRGeneratorLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes

#ifndef __vtkSlicerDRRGeneratorLogic_h
#define __vtkSlicerDRRGeneratorLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// STD includes
#include <array>
#include <vector>
#include <cstdlib>
#include <string>

#include <itkeigen/Eigen/Core>

#include "vtkSlicerDRRGeneratorModuleLogicExport.h"

class qMRMLThreeDView;
class vtkImageData;
class vtkMatrix4x4;
class vtkMRMLCameraNode;
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLVolumePropertyNode;
class vtkMRMLVolumeRenderingDisplayNode;
/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_DRRGENERATOR_MODULE_LOGIC_EXPORT vtkSlicerDRRGeneratorLogic : public vtkSlicerModuleLogic
{
 public:
  static vtkSlicerDRRGeneratorLogic* New();
  vtkTypeMacro(vtkSlicerDRRGeneratorLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  using IJKVec = std::vector<std::array<double, 2>>;

  template <typename NodeType>
  static NodeType* getNodeByName(const std::string& nodeName, bool createIfNotExists = false);

  template <typename NodeType>
  static NodeType* getNodeByID(const std::string& nodeID);

  vtkMRMLVolumeRenderingDisplayNode* createVolumeRenderingNode(vtkMRMLScalarVolumeNode* volumeNode);

  void applyDRR(vtkMRMLScalarVolumeNode* volumeNode, vtkMRMLScalarVolumeNode* drrNode, double rotation[3],
                double translation[3], int size[3]);
  void getFiducialPosition(vtkMRMLScalarVolumeNode*, vtkMRMLMarkupsFiducialNode*, IJKVec&);
  void getDRRFromVolumeRendering(vtkMRMLScalarVolumeNode* drrNode);
  void resetRotation();
  void updateVolumePropertyNode(double wl, double ww, double op);

  vtkMRMLVolumePropertyNode* VolumePropertyNode = nullptr;

  void SetThreeDView(qMRMLThreeDView*);

 protected:
  vtkSlicerDRRGeneratorLogic();
  ~vtkSlicerDRRGeneratorLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached
  /// to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void SetupVolumePropertyNode(vtkMRMLVolumePropertyNode* vpn);
  void Rx(double isocenter[3], double angle, Eigen::Matrix4d& out);
  void Ry(double isocenter[3], double angle, Eigen::Matrix4d& out);
  void Rz(double isocenter[3], double angle, Eigen::Matrix4d& out);
  void ConvertEigenToVTK(Eigen::Matrix4d in, vtkMatrix4x4* out);

  vtkMRMLCameraNode* camera;
  qMRMLThreeDView* view;
  double oldRotation[3]{};
  Eigen::Matrix4d currentVolumeRot = Eigen::Matrix4d::Identity();

 private:
  vtkSlicerDRRGeneratorLogic(const vtkSlicerDRRGeneratorLogic&);  // Not implemented
  void operator=(const vtkSlicerDRRGeneratorLogic&);              // Not implemented
};

#include "vtkSlicerDRRGeneratorLogic.hxx"
#endif
