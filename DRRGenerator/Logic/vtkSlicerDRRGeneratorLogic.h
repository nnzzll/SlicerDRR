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
// MRML includes

// STD includes
#include <array>
#include <cstdlib>
#include <string>
#include <vector>

#include "vtkSlicerDRRGeneratorModuleLogicExport.h"
class DRRGenerator;
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLScalarVolumeNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_DRRGENERATOR_MODULE_LOGIC_EXPORT vtkSlicerDRRGeneratorLogic
    : public vtkSlicerModuleLogic
{
 public:
  static vtkSlicerDRRGeneratorLogic* New();
  vtkTypeMacro(vtkSlicerDRRGeneratorLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  typedef std::vector<std::array<double, 2>> IJKVec;
  template <typename NodeType>
  static NodeType* getNodeByName(const std::string& nodeName, bool createIfNotExists = false);

  template <typename NodeType>
  static NodeType* getNodeByID(const std::string& nodeID);

  void applyDRR(vtkMRMLScalarVolumeNode*, vtkMRMLScalarVolumeNode*, double angle, double threshold,
                double scd, double rotation[3], double translation[3], int size[3],
                double spacing[3]);
  void getFiducialPosition(vtkMRMLScalarVolumeNode*, vtkMRMLMarkupsFiducialNode*, IJKVec&);
  std::shared_ptr<DRRGenerator> drrGen;

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

 private:
  vtkSlicerDRRGeneratorLogic(const vtkSlicerDRRGeneratorLogic&);  // Not implemented
  void operator=(const vtkSlicerDRRGeneratorLogic&);              // Not implemented
};

#include "vtkSlicerDRRGeneratorLogic.hxx"
#endif
