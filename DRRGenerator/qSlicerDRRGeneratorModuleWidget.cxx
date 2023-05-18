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

// ctk includes
#include <ctkCollapsibleButton.h>
#include <ctkSliderWidget.h>

// vtk

// logic
#include <vtkSlicerDRRGeneratorLogic.h>

// MRML
#include <vtkMRMLNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewLogic.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVolumePropertyNode.h>
#include <vtkMRMLVolumeRenderingDisplayNode.h>

// Qt includes
#include <QDebug>
#include <QFormLayout>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QStringList>

// Slicer includes
#include <qMRMLNodeComboBox.h>
#include <qMRMLThreeDView.h>
#include <qMRMLThreeDViewControllerWidget.h>
#include <qMRMLThreeDWidget.h>
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
  QPushButton* applyButton;
  QPushButton* threeDViewButton;
  qMRMLThreeDWidget* threeDWidget;

  qSlicerDRRGeneratorModuleWidgetPrivate(qSlicerDRRGeneratorModuleWidget& object);
  ~qSlicerDRRGeneratorModuleWidgetPrivate();
  void onEnterConnection();
  void onExitConnection();
  void setupUi(qSlicerWidget* qSlicerDRRGeneratorModuleWidget);
  void setup3DWidget();
  void hideAllVolumeNode();
  vtkSlicerDRRGeneratorLogic* logic() const;

 private:
  std::vector<QMetaObject::Connection> connects;

 protected:
  qSlicerDRRGeneratorModuleWidget* const q_ptr;
};

qSlicerDRRGeneratorModuleWidgetPrivate::qSlicerDRRGeneratorModuleWidgetPrivate(
    qSlicerDRRGeneratorModuleWidget& object)
    : q_ptr(&object)
{
}

qSlicerDRRGeneratorModuleWidgetPrivate::~qSlicerDRRGeneratorModuleWidgetPrivate()
{
  delete threeDWidget;
}

void qSlicerDRRGeneratorModuleWidgetPrivate::setupUi(qSlicerWidget* qSlicerDRRGeneratorModuleWidget)
{
  if (qSlicerDRRGeneratorModuleWidget->objectName().isEmpty())
    qSlicerDRRGeneratorModuleWidget->setObjectName(
        QString::fromUtf8("qSlicerDRRGeneratorModuleWidget"));
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
  thSlider->setSuffix(" mm");
  drrFormLayout->addRow("Threshold: ", thSlider);

  sizeSlider = new ctkSliderWidget;
  sizeSlider->setSingleStep(64);
  sizeSlider->setDecimals(0);
  sizeSlider->setMinimum(128);
  sizeSlider->setMaximum(1024);
  sizeSlider->setValue(256);
  drrFormLayout->addRow("DRR Size: ", sizeSlider);

  spacingSlider = new ctkSliderWidget;
  spacingSlider->setSingleStep(0.1);
  spacingSlider->setDecimals(1);
  spacingSlider->setMinimum(0.1);
  spacingSlider->setMaximum(5.0);
  spacingSlider->setValue(1.0);
  spacingSlider->setSuffix(" mm");
  drrFormLayout->addRow("DRR Spacing: ", spacingSlider);

  applyButton = new QPushButton("Apply");
  drrFormLayout->addRow(applyButton);

  threeDViewButton = new QPushButton("Show/Hide 3D Widget");
  drrFormLayout->addRow(threeDViewButton);
}

void qSlicerDRRGeneratorModuleWidgetPrivate::setup3DWidget()
{
  std::string layoutName = "DRRView";
  std::string layoutLabel = "DRR";
  vtkSmartPointer<vtkMRMLViewLogic> viewLogic = vtkSmartPointer<vtkMRMLViewLogic>::New();
  viewLogic->SetMRMLScene(qSlicerApplication::application()->mrmlScene());
  auto viewNode = viewLogic->AddViewNode(layoutName.c_str());
  viewNode->SetLayoutLabel(layoutLabel.c_str());
  threeDWidget = new qMRMLThreeDWidget;
  threeDWidget->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  threeDWidget->setMRMLViewNode(viewNode);
  threeDWidget->threeDController()->setVisible(false);
  viewNode->SetBackgroundColor(0.0, 0.0, 0.0);
  viewNode->SetBackgroundColor2(0.0, 0.0, 0.0);
  viewNode->SetBoxVisible(0);
  viewNode->SetAxisLabelsVisible(0);
}

void qSlicerDRRGeneratorModuleWidgetPrivate::hideAllVolumeNode()
{
  auto scene = qSlicerApplication::application()->mrmlScene();
  std::vector<vtkMRMLNode*> vrdNodes;
  scene->GetNodesByClass("vtkMRMLVolumeRenderingDisplayNode", vrdNodes);
  for (auto vrdNode : vrdNodes)
  {
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(vrdNode)->SetVisibility(0);
  }
}

void qSlicerDRRGeneratorModuleWidgetPrivate::onEnterConnection()
{
  Q_Q(qSlicerDRRGeneratorModuleWidget);
  connects.push_back(QObject::connect(applyButton, SIGNAL(clicked(bool)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(drrSelector, SIGNAL(nodeAdded(vtkMRMLNode*)), q,
                                      SLOT(onDRRNodeAdded(vtkMRMLNode*))));
  connects.push_back(QObject::connect(volumeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q,
                                      SLOT(onVolumeSelected(vtkMRMLNode*))));
  connects.push_back(
      QObject::connect(threeDViewButton, SIGNAL(clicked(bool)), q, SLOT(on3DWidgetShow())));
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
  d->setup3DWidget();
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
  qDebug() << __FUNCTION__ << " clicked!";
}

void qSlicerDRRGeneratorModuleWidget::onDRRNodeAdded(vtkMRMLNode* node)
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  qDebug() << __FUNCTION__ << " clicked!";
}

void qSlicerDRRGeneratorModuleWidget::onVolumeSelected(vtkMRMLNode* node)
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  if (!node) return;
  auto app = qSlicerApplication::application();
  vtkMRMLScene* scene = app->mrmlScene();
  scene->StartState(vtkMRMLScene::BatchProcessState);
  auto volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  d->hideAllVolumeNode();
  auto displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      scene->GetFirstNode(volumeNode->GetName(), "vtkMRMLVolumeRenderingDisplayNode"));
  if (displayNode)
  {
    displayNode->SetVisibility(1);
  }
  else
  {
    displayNode = d->logic()->createVolumeRenderingNode(volumeNode);
    auto viewNode = d->threeDWidget->mrmlViewNode();
    displayNode->SetDisplayableOnlyInView(viewNode->GetID());
    displayNode->SetVisibility(1);
  }

  scene->EndState(vtkMRMLScene::BatchProcessState);
}

void qSlicerDRRGeneratorModuleWidget::on3DWidgetShow()
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  qDebug() << __FUNCTION__ << " clicked!";
  flag3D = !flag3D;
  if (flag3D)
  {
    d->threeDWidget->show();
  }
  else
  {
    d->threeDWidget->hide();
  }
}