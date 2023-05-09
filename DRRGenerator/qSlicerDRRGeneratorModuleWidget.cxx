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
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSlicerDRRGeneratorLogic.h>

// MRML
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
  double drrNodeOrigin[3]{0., 0., 0.};
  double drrNodeSpacing[3]{1.0, 1.0, 1.0};

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

  // DRR Parameter widgets
  angleSlider = new ctkSliderWidget;
  angleSlider->setSingleStep(0.5);
  angleSlider->setDecimals(1);
  angleSlider->setMinimum(-180);
  angleSlider->setMaximum(180);
  angleSlider->setValue(0);
  angleSlider->setSuffix(" °");
  drrFormLayout->addRow("Angle: ", angleSlider);

  rxSlider = new ctkSliderWidget;
  rxSlider->setSingleStep(0.5);
  rxSlider->setDecimals(1);
  rxSlider->setMinimum(-90);
  rxSlider->setMaximum(90);
  rxSlider->setValue(0);
  rxSlider->setSuffix(" °");
  drrFormLayout->addRow("Rotation X: ", rxSlider);

  rySlider = new ctkSliderWidget;
  rySlider->setSingleStep(0.5);
  rySlider->setDecimals(1);
  rySlider->setMinimum(-90);
  rySlider->setMaximum(90);
  rySlider->setValue(0);
  rySlider->setSuffix(" °");
  drrFormLayout->addRow("Rotation Y: ", rySlider);

  rzSlider = new ctkSliderWidget;
  rzSlider->setSingleStep(0.5);
  rzSlider->setDecimals(1);
  rzSlider->setMinimum(-90);
  rzSlider->setMaximum(90);
  rzSlider->setValue(0);
  rzSlider->setSuffix(" °");
  drrFormLayout->addRow("Rotation Z: ", rzSlider);

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
  double rotation[3] = {d->rxSlider->value(), d->rySlider->value(), d->rzSlider->value()};
  double translation[3] = {d->txSlider->value(), d->tySlider->value(), d->tzSlider->value()};
  double threshold = d->thSlider->value();
  int size[3] = {(int)d->sizeSlider->value(), (int)d->sizeSlider->value(), 1};
  double spacing[3] = {d->spacingSlider->value(), d->spacingSlider->value(), 1};
  double scd = d->scdSlider->value();
  double angle = d->angleSlider->value();
  d->logic()->applyDRR(volumeNode, drrNode, angle, threshold, scd, rotation, translation, size, spacing);
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
}

void qSlicerDRRGeneratorModuleWidget::onXRaySelected(vtkMRMLNode* node)
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  auto xrayNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  auto compositeNode = d->logic()->getNodeByID<vtkMRMLSliceCompositeNode>("vtkMRMLSliceCompositeNodeRed");
  compositeNode->SetBackgroundVolumeID(xrayNode->GetID());
  xrayNode->GetSpacing(d->drrNodeSpacing);
  xrayNode->GetOrigin(d->drrNodeOrigin);
}

void qSlicerDRRGeneratorModuleWidget::onOpacityChanged(double value)
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  auto compositeNode = d->logic()->getNodeByID<vtkMRMLSliceCompositeNode>("vtkMRMLSliceCompositeNodeRed");
  compositeNode->SetForegroundOpacity(value);
}