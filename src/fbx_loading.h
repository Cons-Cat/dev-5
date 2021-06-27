#include "includes.h"
#include <fbxsdk.h>
#include <liblava/lava.hpp>

fn read_uv(FbxMesh *mesh, int texture_uv_index)->lava::v2;

fn read_mesh(FbxNode *node)->lava::mesh_data;

fn find_fbx_mesh(FbxNode *node)->std::optional<lava::mesh_data>;

typedef struct {
  // std::unique_ptr<FbxNode> node;
  FbxNode *node;
  int parent_index;
  FbxAMatrix transform;
  // glm::mat4x4 transform;
} Joint;

// std::vector<Joint> joints;

typedef struct {
  double time;
  std::vector<glm::mat4x4> joints;
} Keyframe;

typedef struct {
  double duration;
  std::vector<Keyframe> frames;
} AnimationClip;

void find_fbx_poses(FbxNode *node, std::vector<FbxPose *> *poses);

fn fbxvec_to_glmvec(FbxVector4 vec)->glm::vec3;
