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

#ifndef __qSlicerDRRGeneratorModuleWidget_h
#define __qSlicerDRRGeneratorModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

// STD includes
#include <array>
#include <vector>

#include "qSlicerDRRGeneratorModuleExport.h"

class qSlicerDRRGeneratorModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_DRRGENERATOR_EXPORT qSlicerDRRGeneratorModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

 public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerDRRGeneratorModuleWidget(QWidget *parent = 0);
  virtual ~qSlicerDRRGeneratorModuleWidget();

  using IJKVec = std::vector<std::array<double, 2>>;

  void enter() override;
  void exit() override;

 public slots:
  void on3DWidgetShow();
  void onApplyDRR();
  void onOpacityChanged(double);
  void onResetRotation();
  void onXRaySelected(vtkMRMLNode *);
  void onVolumeSelected(vtkMRMLNode *);
  void onVolumePropertyModified();
  void startInteraction();
  void endInteraction();
  void interaction();

 protected:
  QScopedPointer<qSlicerDRRGeneratorModuleWidgetPrivate> d_ptr;

  void setup() override;
  void displayRegistrationPoint(IJKVec &);
  bool flag3D = false;

 private:
  Q_DECLARE_PRIVATE(qSlicerDRRGeneratorModuleWidget);
  Q_DISABLE_COPY(qSlicerDRRGeneratorModuleWidget);
};

#endif
