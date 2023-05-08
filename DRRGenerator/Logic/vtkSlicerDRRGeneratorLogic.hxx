#include "vtkSlicerDRRGeneratorLogic.h"
#include "qSlicerApplication.h"

#include <vtkNew.h>
#include <vtkMRMLScene.h>

template <typename NodeType>
inline NodeType* vtkSlicerDRRGeneratorLogic::getNodeByName(const std::string& nodeName,
                                                           bool createIfNotExists /*=false*/)
{
  if (nodeName.empty())
  {
    std::cout << __FUNCTION__ << ": nodeName empty." << std::endl;
    return nullptr;
  }

  vtkNew<NodeType> tempNode;
  auto mrmlScene = qSlicerApplication::application()->mrmlScene();
  std::string typeName = tempNode->GetClassName();
  auto node = mrmlScene->GetFirstNode(nodeName.c_str(), typeName.c_str());
  if (node)
  {
    return NodeType::SafeDownCast(node);
  }
  else
  {
    std::cout << __FUNCTION__ << ": node named \"" << nodeName << "\" is not found." << std::endl;
    if (createIfNotExists)
      return NodeType::SafeDownCast(mrmlScene->AddNewNodeByClass(typeName, nodeName));
    else
      return nullptr;
  }
}

template <typename NodeType>
inline NodeType* vtkSlicerDRRGeneratorLogic::getNodeByID(const std::string& nodeID)
{
  if (nodeID.empty())
  {
    std::cout << __FUNCTION__ << ": nodeID empty." << std::endl;
    return nullptr;
  }

  auto mrmlScene = qSlicerApplication::application()->mrmlScene();
  auto node = mrmlScene->GetNodeByID(nodeID);
  if (node)
  {
    return NodeType::SafeDownCast(node);
  }
  else
  {
    std::cout << __FUNCTION__ << ": node ID \"" << nodeID << "\" is not found." << std::endl;
    return nullptr;
  }
}
