#include "includes.h"
#include <fbxsdk.h>
#include <liblava/lava.hpp>

typedef struct Joint_t Joint;
typedef struct Keyframe_t Keyframe;
typedef struct AniClip_t AnimationClip;

fn read_uv(FbxMesh *mesh, int texture_uv_index)->lava::v2;

fn read_mesh(FbxNode *node)->lava::mesh_data;

fn find_fbx_mesh(FbxNode *node)->std::optional<lava::mesh_data>;

void find_fbx_poses(FbxNode *node, std::vector<FbxPose *> *poses);

fn fbxvec_to_glmvec(FbxVector4 vec)->glm::vec3;
