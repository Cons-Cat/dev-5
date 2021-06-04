#include <fbxsdk.h>

#include <iostream>
#include <liblava/lava.hpp>

using namespace std;

typedef struct {
  glm::f32vec3 pos;
  glm::f32vec4 col;
  glm::f32vec2 uv;
  glm::f32vec3 norm;
} vert;

void read_mesh(FbxMesh &in_mesh) {
  static size_t const tri_count = in_mesh.GetPolygonCount();
  std::vector<size_t> triangles;
  //
}

int main(int argc, char *argv[]) {
  FbxManager *manager = FbxManager::Create();
  FbxIOSettings *io_settings = FbxIOSettings::Create(manager, IOSROOT);
  manager->SetIOSettings(io_settings);
  FbxImporter *importer = FbxImporter::Create(manager, "");
  const char *filename = "file.fbx";
  bool import_status =
      importer->Initialize(filename, -1, manager->GetIOSettings());
  if (!import_status) {
    throw "Failed to initialize.";
  }
  FbxScene *scene = FbxScene::Create(manager, "scene");
  importer->Import(scene);
  importer->Destroy();
  // scene->
  return 0;
}
