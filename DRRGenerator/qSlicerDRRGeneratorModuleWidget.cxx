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
#include <vector>
#include "DRRGenerator.h"
// ctk includes
#include <ctkCollapsibleButton.h>
#include <ctkSliderWidget.h>

// vtk
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSlicerDRRGeneratorLogic.h>

// MRML
#include <qMRMLLayoutManager.h>
#include <qMRMLSliceWidget.h>
#include <qSlicerLayoutManager.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLMarkupsFiducialDisplayNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>

// Qt includes
#include <QDebug>
#include <QFormLayout>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QStringList>

// Slicer includes
#include <qMRMLNodeComboBox.h>
#include <qSlicerApplication.h>
#include "qSlicerDRRGeneratorModuleWidget.h"
#include "qSlicerWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerDRRGeneratorModuleWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerDRRGeneratorModuleWidget)

 public:
  QVBoxLayout* verticalLayout;
  ctkCollapsibleButton* drrCollapsibleButton;
  QFormLayout* drrFormLayout;
  qMRMLNodeComboBox* volumeSelector;
  qMRMLNodeComboBox* drrSelector;
  qMRMLNodeComboBox* xraySelector;
  qMRMLNodeComboBox* pointSelector;
  qMRMLNodeComboBox* segmentSelector;
  ctkSliderWidget* angleSlider;
  ctkSliderWidget* rxSlider;
  ctkSliderWidget* rySlider;
  ctkSliderWidget* rzSlider;
  ctkSliderWidget* txSlider;
  ctkSliderWidget* tySlider;
  ctkSliderWidget* tzSlider;
  ctkSliderWidget* scdSlider;
  ctkSliderWidget* thSlider;
  ctkSliderWidget* sizeSlider;
  ctkSliderWidget* spacingSlider;
  ctkSliderWidget* opacitySlider;
  QPushButton* applyButton;
  QPushButton* resetButton;
  double drrNodeOrigin[3]{0., 0., 0.};
  double drrNodeSpacing[3]{1.0, 1.0, 1.0};
  int drrNodeSize[3]{256, 256, 1};

  qSlicerDRRGeneratorModuleWidgetPrivate(qSlicerDRRGeneratorModuleWidget& object);
  void onEnterConnection();
  void onExitConnection();
  void setupUi(qSlicerWidget* qSlicerDRRGeneratorModuleWidget);
  vtkSlicerDRRGeneratorLogic* logic() const;

 private:
  std::vector<QMetaObject::Connection> connects;

 protected:
  qSlicerDRRGeneratorModuleWidget* const q_ptr;
};

qSlicerDRRGeneratorModuleWidgetPrivate::qSlicerDRRGeneratorModuleWidgetPrivate(qSlicerDRRGeneratorModuleWidget& object)
    : q_ptr(&object)
{
}

void qSlicerDRRGeneratorModuleWidgetPrivate::setupUi(qSlicerWidget* qSlicerDRRGeneratorModuleWidget)
{
  if (qSlicerDRRGeneratorModuleWidget->objectName().isEmpty())
    qSlicerDRRGeneratorModuleWidget->setObjectName(QString::fromUtf8("qSlicerDRRGeneratorModuleWidget"));
  qSlicerDRRGeneratorModuleWidget->resize(525, 319);
  verticalLayout = new QVBoxLayout(qSlicerDRRGeneratorModuleWidget);
  verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

  drrCollapsibleButton = new ctkCollapsibleButton("DRR Generation");
  verticalLayout->addWidget(drrCollapsibleButton);

  drrFormLayout = new QFormLayout(drrCollapsibleButton);
  // input volume selector
  volumeSelector = new qMRMLNodeComboBox;
  volumeSelector->setNodeTypes(QStringList("vtkMRMLScalarVolumeNode"));
  volumeSelector->setAddEnabled(false);
  volumeSelector->setRemoveEnabled(false);
  volumeSelector->setNoneEnabled(false);
  volumeSelector->setShowHidden(false);
  volumeSelector->setShowChildNodeTypes(false);
  volumeSelector->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  volumeSelector->setToolTip("Select the CT Volume");
  drrFormLayout->addRow("CT Volume: ", volumeSelector);

  // drr selector
  drrSelector = new qMRMLNodeComboBox;
  drrSelector->setNodeTypes(QStringList("vtkMRMLScalarVolumeNode"));
  drrSelector->setAddEnabled(true);
  drrSelector->setRemoveEnabled(true);
  drrSelector->setNoneEnabled(false);
  drrSelector->setShowHidden(false);
  drrSelector->setShowChildNodeTypes(false);
  drrSelector->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  drrSelector->setToolTip("Select the CT Volume");
  drrFormLayout->addRow("DRR: ", drrSelector);

  // background xray selector
  xraySelector = new qMRMLNodeComboBox;
  xraySelector->setNodeTypes(QStringList("vtkMRMLScalarVolumeNode"));
  xraySelector->setAddEnabled(false);
  xraySelector->setRemoveEnabled(false);
  xraySelector->setNoneEnabled(true);
  xraySelector->setShowHidden(false);
  xraySelector->setShowChildNodeTypes(false);
  xraySelector->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  xraySelector->setToolTip("Select the XRay");
  drrFormLayout->addRow("XRay: ", xraySelector);

  // registration point selector
  pointSelector = new qMRMLNodeComboBox;
  pointSelector->setNodeTypes(QStringList("vtkMRMLMarkupsFiducialNode"));
  pointSelector->setAddEnabled(true);
  pointSelector->setRemoveEnabled(true);
  pointSelector->setNoneEnabled(true);
  pointSelector->setShowHidden(false);
  pointSelector->setShowChildNodeTypes(false);
  pointSelector->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  pointSelector->setToolTip("Select the Registration Points");
  drrFormLayout->addRow("Registration Point", pointSelector);

  // Segmentation Node selector
  segmentSelector = new qMRMLNodeComboBox;
  segmentSelector->setNodeTypes(QStringList("vtkMRMLLabelMapVolumeNode"));
  segmentSelector->setAddEnabled(false);
  segmentSelector->setRemoveEnabled(false);
  segmentSelector->setNoneEnabled(false);
  segmentSelector->setShowHidden(false);
  segmentSelector->setShowChildNodeTypes(false);
  segmentSelector->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  segmentSelector->setToolTip("Select the Segmentation Node");
  drrFormLayout->addRow("Spine STL", segmentSelector);

  // DRR Parameter widgets
  angleSlider = new ctkSliderWidget;
  angleSlider->setSingleStep(0.5);
  angleSlider->setDecimals(1);
  angleSlider->setMinimum(-360);
  angleSlider->setMaximum(360);
  angleSlider->setValue(0);
  angleSlider->setSuffix(" °");
  drrFormLayout->addRow("Angle: ", angleSlider);

  rxSlider = new ctkSliderWidget;
  rxSlider->setSingleStep(0.5);
  rxSlider->setDecimals(1);
  rxSlider->setMinimum(-180);
  rxSlider->setMaximum(180);
  rxSlider->setValue(0);
  rxSlider->setSuffix(" °");
  drrFormLayout->addRow("Rotation X: ", rxSlider);

  rySlider = new ctkSliderWidget;
  rySlider->setSingleStep(0.5);
  rySlider->setDecimals(1);
  rySlider->setMinimum(-180);
  rySlider->setMaximum(180);
  rySlider->setValue(0);
  rySlider->setSuffix(" °");
  drrFormLayout->addRow("Rotation Y: ", rySlider);

  rzSlider = new ctkSliderWidget;
  rzSlider->setSingleStep(0.5);
  rzSlider->setDecimals(1);
  rzSlider->setMinimum(-180);
  rzSlider->setMaximum(180);
  rzSlider->setValue(0);
  rzSlider->setSuffix(" °");
  drrFormLayout->addRow("Rotation Z: ", rzSlider);

  resetButton = new QPushButton("Reset Rotation");
  drrFormLayout->addRow(resetButton);

  txSlider = new ctkSliderWidget;
  txSlider->setSingleStep(0.5);
  txSlider->setDecimals(1);
  txSlider->setMinimum(-1000);
  txSlider->setMaximum(1000);
  txSlider->setValue(0);
  txSlider->setSuffix(" mm");
  drrFormLayout->addRow("Translation X: ", txSlider);

  tySlider = new ctkSliderWidget;
  tySlider->setSingleStep(0.5);
  tySlider->setDecimals(1);
  tySlider->setMinimum(-1000);
  tySlider->setMaximum(1000);
  tySlider->setValue(0);
  tySlider->setSuffix(" mm");
  drrFormLayout->addRow("Translation Y: ", tySlider);

  tzSlider = new ctkSliderWidget;
  tzSlider->setSingleStep(0.5);
  tzSlider->setDecimals(1);
  tzSlider->setMinimum(-1000);
  tzSlider->setMaximum(1000);
  tzSlider->setValue(0);
  tzSlider->setSuffix(" mm");
  drrFormLayout->addRow("Translation Z: ", tzSlider);

  scdSlider = new ctkSliderWidget;
  scdSlider->setSingleStep(0.5);
  scdSlider->setDecimals(1);
  scdSlider->setMinimum(100);
  scdSlider->setMaximum(1500);
  scdSlider->setValue(1000);
  scdSlider->setSuffix(" mm");
  drrFormLayout->addRow("Souce To Isocenter: ", scdSlider);

  thSlider = new ctkSliderWidget;
  thSlider->setSingleStep(0.5);
  thSlider->setDecimals(1);
  thSlider->setMinimum(-1024);
  thSlider->setMaximum(1024);
  thSlider->setValue(0);
  thSlider->setSuffix(" Hu");
  drrFormLayout->addRow("Threshold: ", thSlider);

  sizeSlider = new ctkSliderWidget;
  sizeSlider->setSingleStep(64);
  sizeSlider->setDecimals(0);
  sizeSlider->setMinimum(128);
  sizeSlider->setMaximum(1024);
  sizeSlider->setValue(256);
  drrFormLayout->addRow("DRR Size: ", sizeSlider);

  spacingSlider = new ctkSliderWidget;
  spacingSlider->setSingleStep(0.01);
  spacingSlider->setDecimals(2);
  spacingSlider->setMinimum(0.1);
  spacingSlider->setMaximum(5.0);
  spacingSlider->setValue(1.0);
  spacingSlider->setSuffix(" mm");
  drrFormLayout->addRow("DRR Spacing: ", spacingSlider);

  opacitySlider = new ctkSliderWidget;
  opacitySlider->setSingleStep(0.01);
  opacitySlider->setDecimals(2);
  opacitySlider->setMinimum(0);
  opacitySlider->setMaximum(1.0);
  opacitySlider->setValue(0.5);
  drrFormLayout->addRow("DRR Opacity: ", opacitySlider);

  applyButton = new QPushButton("Apply");
  drrFormLayout->addRow(applyButton);
}

void qSlicerDRRGeneratorModuleWidgetPrivate::onEnterConnection()
{
  Q_Q(qSlicerDRRGeneratorModuleWidget);
  connects.push_back(QObject::connect(applyButton, SIGNAL(clicked(bool)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(angleSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(rxSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(rySlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(rzSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(txSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(tySlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(tzSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(tzSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(thSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(scdSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(spacingSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(sizeSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(opacitySlider, SIGNAL(valueChanged(double)), q, SLOT(onOpacityChanged(double))));
  connects.push_back(
      QObject::connect(xraySelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(onXRaySelected(vtkMRMLNode*))));
  connects.push_back(QObject::connect(resetButton, SIGNAL(clicked(bool)), q, SLOT(onResetRotation())));
}

void qSlicerDRRGeneratorModuleWidgetPrivate::onExitConnection()
{
  Q_Q(qSlicerDRRGeneratorModuleWidget);
  for (auto& connection : connects) QObject::disconnect(connection);
  connects.clear();
}

vtkSlicerDRRGeneratorLogic* qSlicerDRRGeneratorModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerDRRGeneratorModuleWidget);
  return vtkSlicerDRRGeneratorLogic::SafeDownCast(q->logic());
}

qSlicerDRRGeneratorModuleWidget::qSlicerDRRGeneratorModuleWidget(QWidget* _parent)
    : Superclass(_parent), d_ptr(new qSlicerDRRGeneratorModuleWidgetPrivate(*this))
{
}

qSlicerDRRGeneratorModuleWidget::~qSlicerDRRGeneratorModuleWidget() {}

void qSlicerDRRGeneratorModuleWidget::setup()
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

void qSlicerDRRGeneratorModuleWidget::enter()
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  d->onEnterConnection();
}

void qSlicerDRRGeneratorModuleWidget::exit()
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  d->onExitConnection();
}

void qSlicerDRRGeneratorModuleWidget::onApplyDRR()
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->volumeSelector->currentNode());
  vtkMRMLScalarVolumeNode* drrNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->drrSelector->currentNode());
  auto labelNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(d->segmentSelector->currentNode());
  double rotation[3] = {d->rxSlider->value(), d->rySlider->value(), d->rzSlider->value()};
  double translation[3] = {d->txSlider->value(), d->tySlider->value(), d->tzSlider->value()};
  double threshold = d->thSlider->value();
  int size[3];
  if (d->xraySelector->currentNode())
  {
    memcpy(size, d->drrNodeSize, 3 * sizeof(int));
  }
  else
  {
    size[0] = size[1] = (int)d->sizeSlider->value();
    size[2] = 1;
  }
  double spacing[3] = {d->spacingSlider->value(), d->spacingSlider->value(), 1};
  double scd = d->scdSlider->value();
  double angle = d->angleSlider->value();
  d->logic()->applyDRR(volumeNode, drrNode, labelNode, angle, threshold, scd, rotation, translation, size, spacing);
  vtkNew<vtkMatrix4x4> IJKToRASDirectionMatrix;
  volumeNode->GetIJKToRASDirectionMatrix(IJKToRASDirectionMatrix);
  drrNode->SetIJKToRASDirectionMatrix(IJKToRASDirectionMatrix);
  drrNode->SetOrigin(d->drrNodeOrigin);
  drrNode->SetSpacing(d->drrNodeSpacing);

  // Show output
  // selectionNode = slicer.app.applicationLogic().GetSelectionNode()
  // selectionNode.SetReferenceActiveVolumeID(outputVolume.GetID())
  // slicer.app.applicationLogic().PropagateVolumeSelection(1)
  auto compositeNode = d->logic()->getNodeByID<vtkMRMLSliceCompositeNode>("vtkMRMLSliceCompositeNodeRed");
  compositeNode->SetForegroundVolumeID(drrNode->GetID());
  compositeNode->SetForegroundOpacity(d->opacitySlider->value());
  if (!d->xraySelector->currentNode()) compositeNode->SetBackgroundVolumeID("");
  auto layoutManager = qSlicerApplication::application()->layoutManager();
  layoutManager->sliceWidget("Red")->fitSliceToBackground();

  // 配准点的投影
  auto pointNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(d->pointSelector->currentNode());
  if (!pointNode) return;
  IJKVec ijkPoints;
  d->logic()->getFiducialPosition(volumeNode, pointNode, ijkPoints);
  this->displayRegistrationPoint(ijkPoints);
}

void qSlicerDRRGeneratorModuleWidget::onXRaySelected(vtkMRMLNode* node)
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  auto xrayNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  auto compositeNode = d->logic()->getNodeByID<vtkMRMLSliceCompositeNode>("vtkMRMLSliceCompositeNodeRed");
  if (!node)
  {
    d->sizeSlider->setDisabled(false);
    compositeNode->SetBackgroundVolumeID("");
    return;
  }
  compositeNode->SetBackgroundVolumeID(xrayNode->GetID());
  xrayNode->GetSpacing(d->drrNodeSpacing);
  xrayNode->GetOrigin(d->drrNodeOrigin);
  xrayNode->GetImageData()->GetDimensions(d->drrNodeSize);
  d->sizeSlider->setValue(d->drrNodeSize[0]);
  d->sizeSlider->setDisabled(true);
}

void qSlicerDRRGeneratorModuleWidget::onOpacityChanged(double value)
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  auto compositeNode = d->logic()->getNodeByID<vtkMRMLSliceCompositeNode>("vtkMRMLSliceCompositeNodeRed");
  compositeNode->SetForegroundOpacity(value);
}

void qSlicerDRRGeneratorModuleWidget::displayRegistrationPoint(IJKVec& ijkPoints)
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  auto registNode = d->logic()->getNodeByName<vtkMRMLMarkupsFiducialNode>("RegisterPoints1", true);
  registNode->SetLocked(1);
  auto displayNode = vtkMRMLMarkupsFiducialDisplayNode::SafeDownCast(registNode->GetDisplayNode());
  displayNode->SetDisplayableOnlyInView("vtkMRMLSliceNodeRed");
  displayNode->SetGlyphScale(1.0);
  auto drrNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->drrSelector->currentNode());
  if (!drrNode) return;
  vtkNew<vtkMatrix4x4> IJKToRAS;
  drrNode->GetIJKToRASMatrix(IJKToRAS);
  registNode->RemoveAllControlPoints();
  double ijkPos[4]{0, 0, 0, 1}, rasPos[4]{0, 0, 0, 1};
  for (size_t i = 0; i < ijkPoints.size(); i++)
  {
    ijkPos[0] = ijkPoints[i][0];
    ijkPos[1] = ijkPoints[i][1];
    IJKToRAS->MultiplyPoint(ijkPos, rasPos);
    registNode->AddControlPoint(rasPos, std::to_string(i + 1));
  }
}

void qSlicerDRRGeneratorModuleWidget::onResetRotation()
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  d->onExitConnection();
  d->rxSlider->setValue(0);
  d->rySlider->setValue(0);
  d->rzSlider->setValue(0);
  d->logic()->resetDRRGenerator();
  this->onApplyDRR();
  d->onEnterConnection();
}