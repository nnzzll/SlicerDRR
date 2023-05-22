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
#include "qSlicerDRRGeneratorModuleWidget.h"

#include <vector>

// ctk includes
#include <ctkCollapsibleButton.h>
#include <ctkDoubleSlider.h>
#include <ctkSliderWidget.h>

// vtk
#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkVolumeProperty.h>

// logic
#include <vtkSlicerDRRGeneratorLogic.h>

// MRML
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLMarkupsFiducialDisplayNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
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
#include <qMRMLSliceWidget.h>
#include <qMRMLThreeDView.h>
#include <qMRMLThreeDViewControllerWidget.h>
#include <qMRMLThreeDWidget.h>
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerWidget.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerDRRGeneratorModuleWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerDRRGeneratorModuleWidget)

 public:
  QVBoxLayout* verticalLayout;
  ctkCollapsibleButton* drrCollapsibleButton;
  ctkCollapsibleButton* vpCollapsibleButton;
  QFormLayout *drrFormLayout, *vpFormLayout;
  qMRMLNodeComboBox* volumeSelector;
  qMRMLNodeComboBox* drrSelector;
  qMRMLNodeComboBox* xraySelector;
  qMRMLNodeComboBox* pointSelector;
  ctkSliderWidget* rxSlider;
  ctkSliderWidget* rySlider;
  ctkSliderWidget* rzSlider;
  ctkSliderWidget* txSlider;
  ctkSliderWidget* tySlider;
  ctkSliderWidget* tzSlider;
  ctkSliderWidget* sizeSlider;
  ctkSliderWidget* opacitySlider;
  ctkSliderWidget* wwSlider;  // DRR体绘制的窗宽
  ctkSliderWidget* wlSlider;  // DRR体绘制的窗位
  ctkSliderWidget* opSlider;  // DRR体绘制的透明度
  QPushButton* applyButton;
  QPushButton* resetButton;
  QPushButton* threeDViewButton;
  qMRMLThreeDWidget* threeDWidget;

  double drrNodeOrigin[3]{0., 0., 0.};
  double drrNodeSpacing[3]{1.0, 1.0, 1.0};
  int drrNodeSize[3]{256, 256, 1};

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

qSlicerDRRGeneratorModuleWidgetPrivate::qSlicerDRRGeneratorModuleWidgetPrivate(qSlicerDRRGeneratorModuleWidget& object)
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
    qSlicerDRRGeneratorModuleWidget->setObjectName(QString::fromUtf8("qSlicerDRRGeneratorModuleWidget"));
  qSlicerDRRGeneratorModuleWidget->resize(525, 319);
  verticalLayout = new QVBoxLayout(qSlicerDRRGeneratorModuleWidget);
  verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

  drrCollapsibleButton = new ctkCollapsibleButton("DRR Generation");
  verticalLayout->addWidget(drrCollapsibleButton);

  vpCollapsibleButton = new ctkCollapsibleButton("Volume Rendering Property");
  verticalLayout->addWidget(vpCollapsibleButton);

  drrFormLayout = new QFormLayout(drrCollapsibleButton);
  vpFormLayout = new QFormLayout(vpCollapsibleButton);
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

  // xray selector
  xraySelector = new qMRMLNodeComboBox;
  xraySelector->setNodeTypes(QStringList("vtkMRMLScalarVolumeNode"));
  xraySelector->setAddEnabled(false);
  xraySelector->setRemoveEnabled(false);
  xraySelector->setNoneEnabled(true);
  xraySelector->setShowHidden(false);
  xraySelector->setShowChildNodeTypes(false);
  xraySelector->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  xraySelector->setToolTip("Select XRay");
  drrFormLayout->addRow("X-Ray: ", xraySelector);

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

  sizeSlider = new ctkSliderWidget;
  sizeSlider->setSingleStep(64);
  sizeSlider->setDecimals(0);
  sizeSlider->setMinimum(128);
  sizeSlider->setMaximum(1024);
  sizeSlider->setValue(256);
  drrFormLayout->addRow("DRR Size: ", sizeSlider);

  opacitySlider = new ctkSliderWidget;
  opacitySlider->setSingleStep(0.01);
  opacitySlider->setDecimals(2);
  opacitySlider->setMinimum(0);
  opacitySlider->setMaximum(1.0);
  opacitySlider->setValue(0.5);
  drrFormLayout->addRow("DRR Opacity: ", opacitySlider);

  wlSlider = new ctkSliderWidget;
  wlSlider->setSingleStep(1);
  wlSlider->setDecimals(0);
  wlSlider->setMinimum(-1024);
  wlSlider->setMaximum(1024);
  wlSlider->setValue(790);
  vpFormLayout->addRow("Window Level: ", wlSlider);

  wwSlider = new ctkSliderWidget;
  wwSlider->setSingleStep(2);
  wwSlider->setDecimals(0);
  wwSlider->setMinimum(2);
  wwSlider->setMaximum(2048);
  wwSlider->setValue(1286);
  vpFormLayout->addRow("Window Wide: ", wwSlider);

  opSlider = new ctkSliderWidget;
  opSlider->setSingleStep(0.01);
  opSlider->setDecimals(2);
  opSlider->setMinimum(0);
  opSlider->setMaximum(1);
  opSlider->setValue(0.30);
  vpFormLayout->addRow("Scalar Opacity: ", opSlider);

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
  auto view = threeDWidget->threeDView();
  view->setFixedSize(256, 256);
  threeDWidget->show();
  threeDWidget->hide();
  viewNode->SetBackgroundColor(0.0, 0.0, 0.0);
  viewNode->SetBackgroundColor2(0.0, 0.0, 0.0);
  viewNode->SetBoxVisible(0);
  viewNode->SetAxisLabelsVisible(0);
  this->logic()->SetThreeDView(view);
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
  connects.push_back(QObject::connect(rxSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(rySlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(rzSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(txSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(tySlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(tzSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(sizeSlider, SIGNAL(valueChanged(double)), q, SLOT(onApplyDRR())));
  connects.push_back(QObject::connect(opacitySlider, SIGNAL(valueChanged(double)), q, SLOT(onOpacityChanged(double))));
  connects.push_back(QObject::connect(pointSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q,
                                      SLOT(onPointSelected(vtkMRMLNode*))));
  connects.push_back(QObject::connect(volumeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q,
                                      SLOT(onVolumeSelected(vtkMRMLNode*))));
  connects.push_back(
      QObject::connect(xraySelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(onXRaySelected(vtkMRMLNode*))));
  connects.push_back(QObject::connect(resetButton, SIGNAL(clicked(bool)), q, SLOT(onResetRotation())));

  connects.push_back(QObject::connect(threeDViewButton, SIGNAL(clicked(bool)), q, SLOT(on3DWidgetShow())));

  // 体绘制参数设置
  connects.push_back(QObject::connect(wwSlider, SIGNAL(valueChanged(double)), q, SLOT(onVolumePropertyModified())));
  connects.push_back(QObject::connect(wlSlider, SIGNAL(valueChanged(double)), q, SLOT(onVolumePropertyModified())));
  connects.push_back(QObject::connect(opSlider, SIGNAL(valueChanged(double)), q, SLOT(onVolumePropertyModified())));

  // 调整渲染效果时使变化更平滑
  connects.push_back(QObject::connect(wwSlider->slider(), SIGNAL(sliderPressed()), q, SLOT(startInteraction())));
  connects.push_back(QObject::connect(wlSlider->slider(), SIGNAL(sliderPressed()), q, SLOT(startInteraction())));
  connects.push_back(QObject::connect(opSlider->slider(), SIGNAL(sliderPressed()), q, SLOT(startInteraction())));
  connects.push_back(QObject::connect(wwSlider->slider(), SIGNAL(sliderReleased()), q, SLOT(endInteraction())));
  connects.push_back(QObject::connect(wlSlider->slider(), SIGNAL(sliderReleased()), q, SLOT(endInteraction())));
  connects.push_back(QObject::connect(opSlider->slider(), SIGNAL(sliderReleased()), q, SLOT(endInteraction())));
  connects.push_back(QObject::connect(wwSlider, SIGNAL(valueChanged(double)), q, SLOT(interaction())));
  connects.push_back(QObject::connect(wlSlider, SIGNAL(valueChanged(double)), q, SLOT(interaction())));
  connects.push_back(QObject::connect(opSlider, SIGNAL(valueChanged(double)), q, SLOT(interaction())));
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
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->volumeSelector->currentNode());
  vtkMRMLScalarVolumeNode* drrNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->drrSelector->currentNode());
  if (!volumeNode || !drrNode) return;
  double rotation[3] = {d->rxSlider->value(), d->rySlider->value(), d->rzSlider->value()};
  double translation[3] = {d->txSlider->value(), d->tySlider->value(), d->tzSlider->value()};
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

  d->logic()->applyDRR(volumeNode, drrNode, rotation, translation, size);
  vtkNew<vtkMatrix4x4> IJKToRASDirectionMatrix;
  volumeNode->GetIJKToRASDirectionMatrix(IJKToRASDirectionMatrix);
  drrNode->SetIJKToRASDirectionMatrix(IJKToRASDirectionMatrix);
  drrNode->SetOrigin(d->drrNodeOrigin);
  drrNode->SetSpacing(d->drrNodeSpacing);

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

void qSlicerDRRGeneratorModuleWidget::onOpacityChanged(double value)
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  auto compositeNode = d->logic()->getNodeByID<vtkMRMLSliceCompositeNode>("vtkMRMLSliceCompositeNodeRed");
  compositeNode->SetForegroundOpacity(value);
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
  auto transformNode = d->logic()->getNodeByName<vtkMRMLLinearTransformNode>("VolumeTransform", true);
  volumeNode->SetAndObserveTransformNodeID(transformNode->GetID());
  d->threeDWidget->threeDView()->resetCamera();
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

void qSlicerDRRGeneratorModuleWidget::onResetRotation()
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  d->onExitConnection();
  d->rxSlider->setValue(0);
  d->rySlider->setValue(0);
  d->rzSlider->setValue(0);
  d->logic()->resetRotation();
  this->onApplyDRR();
  d->onEnterConnection();
}

void qSlicerDRRGeneratorModuleWidget::onPointSelected(vtkMRMLNode* node)
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  auto pointNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(node);
  if (!pointNode) return;
  auto transformNode = d->logic()->getNodeByName<vtkMRMLLinearTransformNode>("VolumeTransform", true);
  pointNode->SetAndObserveTransformNodeID(transformNode->GetID());
  pointNode->GetDisplayNode()->SetVisibility(0);
}

void qSlicerDRRGeneratorModuleWidget::onVolumePropertyModified()
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  double wl = d->wlSlider->value();
  double ww = d->wwSlider->value();
  double op = d->opSlider->value();
  d->logic()->updateVolumePropertyNode(wl, ww, op);
  auto drrNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->drrSelector->currentNode());
  if (drrNode)
  {
    d->logic()->getDRRFromVolumeRendering(drrNode);
  }
}

void qSlicerDRRGeneratorModuleWidget::startInteraction()
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  if (!d->logic()->VolumePropertyNode)
  {
    return;
  }

  vtkVolumeProperty* volumeProperty = d->logic()->VolumePropertyNode->GetVolumeProperty();
  if (volumeProperty)
  {
    volumeProperty->InvokeEvent(vtkCommand::StartInteractionEvent);
  }
}

void qSlicerDRRGeneratorModuleWidget::endInteraction()
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  if (!d->logic()->VolumePropertyNode)
  {
    return;
  }

  vtkVolumeProperty* volumeProperty = d->logic()->VolumePropertyNode->GetVolumeProperty();
  if (volumeProperty)
  {
    volumeProperty->InvokeEvent(vtkCommand::EndInteractionEvent);
  }
}

void qSlicerDRRGeneratorModuleWidget::interaction()
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  if (!d->logic()->VolumePropertyNode)
  {
    return;
  }

  vtkVolumeProperty* volumeProperty = d->logic()->VolumePropertyNode->GetVolumeProperty();
  if (volumeProperty)
  {
    volumeProperty->InvokeEvent(vtkCommand::InteractionEvent);
  }
}

void qSlicerDRRGeneratorModuleWidget::displayRegistrationPoint(IJKVec& ijkPoints)
{
  Q_D(qSlicerDRRGeneratorModuleWidget);
  auto drrNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->drrSelector->currentNode());
  if (!drrNode) return;
  auto registNode = d->logic()->getNodeByName<vtkMRMLMarkupsFiducialNode>("RegisterPoints", true);
  registNode->SetLocked(1);
  auto displayNode = vtkMRMLMarkupsFiducialDisplayNode::SafeDownCast(registNode->GetDisplayNode());
  displayNode->SetDisplayableOnlyInView("vtkMRMLSliceNodeRed");
  displayNode->SetGlyphScale(1.0);

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