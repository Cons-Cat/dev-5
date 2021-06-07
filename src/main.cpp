#include <cstddef>
#include <fbxsdk.h>
#include <iostream>
#include <liblava/lava.hpp>

#define success(x, str)                                                        \
  if (!x) {                                                                    \
    std::cout << str << std::endl;                                             \
  }

#define fn auto

fn read_mesh(FbxNode *node)->std::vector<lava::vertex> {
  std::vector<lava::vertex> verts_output;
  FbxMesh *mesh = node->GetMesh();
  FbxSkin *skin = (FbxSkin *)mesh->GetDeformer(0, FbxDeformer::eSkin);
  size_t tri_count = mesh->GetPolygonCount();
  FbxVector4 *ctrl_points = mesh->GetControlPoints();
  for (size_t i = 0; i < tri_count; i++) {
    for (size_t j = 0; j < 3; j++) {
      verts_output.push_back(lava::vertex{
          .position =
              lava::v3{
                  static_cast<float>(
                      ctrl_points[mesh->GetPolygonVertex(i, j)][0]),
                  static_cast<float>(
                      ctrl_points[mesh->GetPolygonVertex(i, j)][1]),
                  static_cast<float>(
                      ctrl_points[mesh->GetPolygonVertex(i, j)][2]),
              },
      });
      // TODO: Other fields need to be read.
    }
  }
  return verts_output;
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
