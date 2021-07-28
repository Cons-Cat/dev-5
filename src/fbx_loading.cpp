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
  lava::mesh_template_data output;
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

fn fbxvec_to_glmvec(FbxVector4 vec)->glm::vec3 {
  return glm::vec3(
      static_cast<glm::vec4>(*reinterpret_cast<glm::dvec4 *>(&vec)));
}

fn rowmaj_to_colmaj(lava::mat4 row_mat)->lava::mat4 {
  lava::mat4 col_mat = lava::mat4(1);
  // for (size_t i = 3; i < 4; i++) {
  //   for (size_t j = 3; j < 4; j++) {
  col_mat[3][0] = row_mat[0][3];
  col_mat[3][1] = row_mat[1][3];
  col_mat[3][2] = row_mat[2][3];
  //   }
  // }
  // col_mat[3] = {0, 0, 0, 1};
  for (size_t i = 0; i < 3; i++) {
    // col_mat[i][3] = row_mat[i][3];
  }
  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < 3; j++) {
      // col_mat[i][j] = row_mat[i][j];
    }
  }
  return col_mat;
}

fn fbxmat_to_lavamat(FbxAMatrix fbx_mat)->lava::mat4 {
  lava::mat4 lava_mat;
  // for (size_t i = 0; i < 4; i++) {
  //   for (size_t j = 0; j < 4; j++) {
  //     lava_mat[i][j] = fbx_mat.Get(i, j);
  //   }
  // }
  lava_mat = static_cast<lava::mat4>(*reinterpret_cast<glm::dmat4
  *>(&fbx_mat));
  return // rowmaj_to_colmaj
    (lava_mat);
}
