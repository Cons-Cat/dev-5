#include "fbx_loading.h"

using fbxsdk::FbxNode;

fn read_uv(FbxMesh *mesh, int texture_uv_index)->lava::v2 {
  auto uv = lava::v2();
  FbxGeometryElementUV *vertex_uv = mesh->GetElementUV();
  uv.x = (float)vertex_uv->GetDirectArray().GetAt(texture_uv_index)[0];
  uv.y = (float)vertex_uv->GetDirectArray().GetAt(texture_uv_index)[1];
  return uv;
}

fn read_mesh(FbxNode *node)->lava::mesh_data {
  lava::mesh_data output;
  FbxMesh *mesh = node->GetMesh();
  FbxSkin *skin = (FbxSkin *)mesh->GetDeformer(0, FbxDeformer::eSkin);
  size_t tri_count = mesh->GetPolygonCount();
  FbxVector4 *ctrl_points = mesh->GetControlPoints();
  for (size_t i = 0; i < tri_count; i++) {
    for (size_t j = 0; j < 3; j++) {
      size_t ctrl_index = mesh->GetPolygonVertex(i, j);
      output.vertices.push_back(
          lava::vertex{.position =
                           lava::v3{
                               static_cast<float>(ctrl_points[ctrl_index][0]),
                               static_cast<float>(ctrl_points[ctrl_index][1]),
                               static_cast<float>(ctrl_points[ctrl_index][2]),
                           },
                       .color = lava::v4{1, 1, 1, 1},
                       .uv = read_uv(mesh, mesh->GetTextureUVIndex(i, j)),
                       .normal = lava::v3{
                           static_cast<float>(
                               mesh->GetElementNormal()->GetDirectArray().GetAt(
                                   ctrl_index)[0]),
                           static_cast<float>(
                               mesh->GetElementNormal()->GetDirectArray().GetAt(
                                   ctrl_index)[1]),
                           static_cast<float>(
                               mesh->GetElementNormal()->GetDirectArray().GetAt(
                                   ctrl_index)[2]),
                       }});
      // Mirror UVs.
      output.vertices[output.vertices.size() - 1].uv =
          lava::v2{output.vertices[output.vertices.size() - 1].uv.x,
                   -output.vertices[output.vertices.size() - 1].uv.y};
    }
  }
  return output;
}

fn find_fbx_mesh(FbxNode *node)->std::optional<lava::mesh_data> {
  FbxNodeAttribute *attribute = node->GetNodeAttribute();
  if (attribute != nullptr) {
    if (attribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
      return read_mesh(node);
    }
  }
  for (size_t i = 0; i < node->GetChildCount(); i++) {
    auto maybe_mesh = find_fbx_mesh(node->GetChild(i));
    if (maybe_mesh.has_value()) {
      return maybe_mesh;
    }
  }
  return std::nullopt;
}

void find_fbx_poses(FbxNode *node, std::vector<FbxPose *> *poses)
// ->std::optional<FbxPose *>
{
  if (node->GetNodeAttribute()) {
    if (node->GetNodeAttribute()->GetAttributeType()) {
      if (node->GetNodeAttribute()->GetAttributeType() ==
          FbxNodeAttribute::eSkeleton) {
        poses->push_back((FbxPose *)node);
      }
    }
  }
  for (size_t i = 0; i < node->GetChildCount(); i++) {
    FbxNode *child = node->GetChild(i);
    find_fbx_poses(child, poses);
  }
}
