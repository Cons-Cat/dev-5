#include <fbxsdk.h>
// #include <fbxsdk/fbxsdk_def.h>

// #include <glm/glm.hpp>
#include <iostream>
#include <liblava/lava.hpp>

#define success(x, str)                                                        \
  if (!x) {                                                                    \
    std::cout << str << std::endl;                                             \
  }

int main(int argc, char *argv[]) {
  // TODO: Initialize the path.
  std::string path = "file.fbx";
  FbxManager *manager = FbxManager::Create();
  FbxIOSettings *io_settings = FbxIOSettings::Create(manager, IOSROOT);
  manager->SetIOSettings(io_settings);
  FbxImporter *importer = FbxImporter::Create(manager, "");
  FbxScene *scene = FbxScene::Create(manager, "");
  success(importer->Initialize(path.c_str(), -1, manager->GetIOSettings()),
          "Failed to import");
  importer->Import(scene);
  importer->Destroy();
  // // scene->
  std::cout << "Path: " << path;
  return 0;
}
