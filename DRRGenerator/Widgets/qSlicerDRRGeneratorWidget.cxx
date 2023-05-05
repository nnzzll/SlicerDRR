/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/
// Slicer includes
#include <qSlicerApplication.h>
#include <vtkMRMLScene.h>

// Widgets includes
#include <qMRMLNodeComboBox.h>
#include "qSlicerDRRGeneratorWidget.h"

// ctk includes
#include <ctkCollapsibleButton.h>
#include <ctkSliderWidget.h>

// Qt includes
#include <QDebug>
#include <QFormLayout>
#include <QPushButton>
#include <QString>
#include <QStringList>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_DRRGenerator
class qSlicerDRRGeneratorWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerDRRGeneratorWidget);

 protected:
  qSlicerDRRGeneratorWidget* const q_ptr;

 public:
  qSlicerDRRGeneratorWidgetPrivate(qSlicerDRRGeneratorWidget& object);
  void setupUi(qSlicerDRRGeneratorWidget*);

  ctkCollapsibleButton* drrCollapsibleButton;
  QFormLayout* drrFormLayout;
  qMRMLNodeComboBox* volumeSelector;
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
};

// --------------------------------------------------------------------------
qSlicerDRRGeneratorWidgetPrivate ::qSlicerDRRGeneratorWidgetPrivate(
    qSlicerDRRGeneratorWidget& object)
    : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerDRRGeneratorWidgetPrivate ::setupUi(qSlicerDRRGeneratorWidget* widget)
{
  drrCollapsibleButton = new ctkCollapsibleButton("DRR Generation", widget);
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
  drrFormLayout->addRow("CT Volume", volumeSelector);

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
  scdSlider->setMinimum(-90);
  scdSlider->setMaximum(90);
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
}

//-----------------------------------------------------------------------------
// qSlicerDRRGeneratorWidget methods

//-----------------------------------------------------------------------------
qSlicerDRRGeneratorWidget ::qSlicerDRRGeneratorWidget(QWidget* parentWidget)
    : Superclass(parentWidget), d_ptr(new qSlicerDRRGeneratorWidgetPrivate(*this))
{
  Q_D(qSlicerDRRGeneratorWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerDRRGeneratorWidget ::~qSlicerDRRGeneratorWidget() {}
