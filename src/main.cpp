#include <cstddef>
#include <fbxsdk.h>
#include <iostream>
#include <liblava/lava.hpp>
#include <typeinfo>

#define success(x, str)                                                        \
  if (!x) {                                                                    \
    std::cout << str << std::endl;                                             \
  }

#define fn auto

fn read_mesh(FbxNode *node)->lava::mesh_data {
  lava::mesh_data output;
  FbxMesh *mesh = node->GetMesh();
  FbxSkin *skin = (FbxSkin *)mesh->GetDeformer(0, FbxDeformer::eSkin);
  size_t tri_count = mesh->GetPolygonCount();
  FbxVector4 *ctrl_points = mesh->GetControlPoints();
  for (size_t i = 0; i < tri_count; i++) {
    for (size_t j = 0; j < 3; j++) {
      output.vertices.push_back(lava::vertex{
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
      // TODO: Other vertex fields need to be read.
    }
  }
  return output;
}

template <typename T> fn traverse_node(FbxNode *node)->T {
  FbxNodeAttribute *attribute = node->GetNodeAttribute();
  switch (attribute->GetAttributeType()) {
  case FbxNodeAttribute::eMesh:
    return read_mesh(node);
    break;
  default:
    break;
  }
  for (size_t i = 0; i < node->GetChildCount(); i++) {
    traverse_node<T>(node->GetChild(i));
  }
}

int main(int argc, char *argv[]) {
  // Load and read the mesh from an FBX.
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
  lava::mesh_data loaded_data = traverse_node<lava::mesh_data>(root_node);
  std::cout << "Path: " << path;
  // Render the mesh.
  lava::app app("DEV 5 - WGooch", {argc, argv});
  app.setup();
  lava::mesh::ptr mesh = lava::make_mesh();
  mesh->add_data(loaded_data);
  lava::texture::ptr default_texture =
      create_default_texture(app.device, {4096, 4096});
  app.staging.add(default_texture);
  app.camera.position = lava::v3(0.832f, 0.036f, 2.304f);
  app.camera.rotation = lava::v3(8.42f, -29.73f, 0.f);
  glm::mat4 model_space = glm::identity<lava::mat4>();
  lava::buffer model_buffer;
  model_buffer.create_mapped(app.device, &model_space,
                             sizeof(typeid(model_space)),
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  lava::graphics_pipeline::ptr pipeline;
  lava::pipeline_layout::ptr layout;
  lava::descriptor::ptr descriptor;
  lava::descriptor::pool::ptr descriptor_pool;
  VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
  app.on_create = [&]() {
    pipeline = make_graphics_pipeline(app.device);
    if (!pipeline->add_shader(lava::file_data("../res/vert.spirv"),
                              VK_SHADER_STAGE_VERTEX_BIT))
      return false;

    if (!pipeline->add_shader(lava::file_data("../res/frag.spirv"),
                              VK_SHADER_STAGE_FRAGMENT_BIT))
      return false;
    pipeline->add_color_blend_attachment();
    pipeline->set_depth_test_and_write();
    pipeline->set_depth_compare_op(VK_COMPARE_OP_LESS_OR_EQUAL);
    pipeline->set_vertex_input_binding(
        {0, sizeof(lava::vertex), VK_VERTEX_INPUT_RATE_VERTEX});
    pipeline->set_vertex_input_attributes({
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT,
         lava::to_ui32(offsetof(lava::vertex, position))},
        {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
         lava::to_ui32(offsetof(lava::vertex, color))},
        {2, 0, VK_FORMAT_R32G32_SFLOAT,
         lava::to_ui32(offsetof(lava::vertex, uv))},
    });
    descriptor = lava::make_descriptor();
    descriptor->add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            VK_SHADER_STAGE_VERTEX_BIT);
    descriptor->add_binding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            VK_SHADER_STAGE_VERTEX_BIT);
    descriptor->add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            VK_SHADER_STAGE_FRAGMENT_BIT);
    if (!descriptor->create(app.device))
      return false;
    descriptor_pool = lava::make_descriptor_pool();
    if (!descriptor_pool->create(
            app.device, {
                            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
                            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},
                        }))
      return false;
    layout = lava::make_pipeline_layout();
    layout->add(descriptor);
    if (!layout->create(app.device))
      return false;
    pipeline->set_layout(layout);
    descriptor_set = descriptor->allocate(descriptor_pool->get());
    VkWriteDescriptorSet const write_desc_ubo_camera{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor_set,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = app.camera.get_descriptor_info(),
    };
    VkWriteDescriptorSet const write_desc_ubo_spawn{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor_set,
        .dstBinding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = model_buffer.get_descriptor_info(),
    };
    VkWriteDescriptorSet const write_desc_sampler{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor_set,
        .dstBinding = 2,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = default_texture->get_descriptor_info(),
    };
    app.device->vkUpdateDescriptorSets(
        {write_desc_ubo_camera, write_desc_ubo_spawn, write_desc_sampler});
    pipeline->on_process = [&](VkCommandBuffer cmd_buf) {
      layout->bind(cmd_buf, descriptor_set);
      mesh->bind_draw(cmd_buf);
    };
    lava::render_pass::ptr render_pass = app.shading.get_pass();
    if (!pipeline->create(render_pass->get()))
      return false;
    render_pass->add_front(pipeline);
    return true;
  };
  return EXIT_SUCCESS;
}
