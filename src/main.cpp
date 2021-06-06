#include <fbxsdk.h>
// #include <fbxsdk/fbxsdk_def.h>

// #include <glm/glm.hpp>
#include <iostream>
#include <liblava/lava.hpp>

#define success(x, str)                                                        \
  if (!x) {                                                                    \
    std::cout << str << std::endl;                                             \
  }

void traverse_node(FbxNode *node) {
  FbxNodeAttribute *attribute = node->GetNodeAttribute();
  switch (attribute->GetAttributeType()) {
  case FbxNodeAttribute::eMesh:
    // TODO: Deserialize mest.
    break;
  default:
    break;
  }
  for (size_t i = 0; i < node->GetChildCount(); i++) {
    traverse_node(node->GetChild(i));
  }
}

int main(int argc, char *argv[]) {
  // TODO: Initialize the path.
  std::string path = "../res/Idle.fbx";
  FbxManager *manager = FbxManager::Create();
  FbxIOSettings *io_settings = FbxIOSettings::Create(manager, IOSROOT);
  manager->SetIOSettings(io_settings);
  FbxImporter *importer = FbxImporter::Create(manager, "");
  FbxScene *scene = FbxScene::Create(manager, "");
  success(importer->Initialize(path.c_str(), -1, manager->GetIOSettings()),
          "Failed to import");
  importer->Import(scene);
  importer->Destroy();
  FbxNode *root_node = scene->GetRootNode();
  traverse_node(root_node);
  std::cout << "Path: " << path;
  return 0;
}
