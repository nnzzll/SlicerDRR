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
#include <vtkSlicerDRRGeneratorLogic.h>

// DRRGenerator includes
#include "qSlicerDRRGeneratorModule.h"
#include "qSlicerDRRGeneratorModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerDRRGeneratorModulePrivate
{
public:
  qSlicerDRRGeneratorModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerDRRGeneratorModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerDRRGeneratorModulePrivate::qSlicerDRRGeneratorModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerDRRGeneratorModule methods

//-----------------------------------------------------------------------------
qSlicerDRRGeneratorModule::qSlicerDRRGeneratorModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerDRRGeneratorModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerDRRGeneratorModule::~qSlicerDRRGeneratorModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerDRRGeneratorModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerDRRGeneratorModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerDRRGeneratorModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerDRRGeneratorModule::icon() const
{
  return QIcon(":/Icons/DRRGenerator.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerDRRGeneratorModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerDRRGeneratorModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerDRRGeneratorModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerDRRGeneratorModule
::createWidgetRepresentation()
{
  return new qSlicerDRRGeneratorModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerDRRGeneratorModule::createLogic()
{
  return vtkSlicerDRRGeneratorLogic::New();
}
