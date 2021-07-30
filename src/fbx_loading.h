#include <fbxsdk.h>

#include <liblava/lava.hpp>

#include "includes.h"

typedef struct {
  lava::v3 position;
  lava::v4 color;
  lava::v2 uv;
  lava::v3 normal;
  std::array<std::uint32_t, 4> weight_indices;
  lava::v4 bone_weights;
} skin_vertex;

fn read_uv(FbxMesh *mesh, int texture_uv_index)->lava::v2;

fn read_mesh(FbxNode *node)->lava::mesh_template_data<skin_vertex>;

fn find_fbx_mesh(FbxNode *node)
    ->std::optional<lava::mesh_template_data<skin_vertex>>;

// TODO: Is the matrix here redundant?
typedef struct {
  FbxNode *node;
  int parent_index;
  FbxAMatrix transform;
} Joint;

typedef struct {
  lava::v3 translation;
  alignas(16) glm::quat orientation;
} Transform;

typedef struct {
  double time;
  std::vector<Transform> transforms;
} Keyframe;

typedef struct {
  double duration;
  std::vector<Keyframe> frames;
} AnimationClip;

void find_fbx_poses(FbxNode *node, std::vector<FbxPose *> *poses);

fn fbxvec_to_glmvec(FbxVector4 vec)->glm::vec3;

fn fbxmat_to_lavamat(FbxAMatrix mat)->lava::mat4;
