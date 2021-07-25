#include "pipelines.h"

#include <iostream>

fn create_mesh_descriptor_layout(lava::app& app)->lava::descriptor::ptr {
  lava::descriptor::ptr descriptor_layout = lava::make_descriptor();

  // Camera position float3 and View-Proj matrix
  lava::descriptor::binding::ptr global_binding =
      lava::make_descriptor_binding(0);
  global_binding->set_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  global_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT |
                                  VK_SHADER_STAGE_FRAGMENT_BIT);
  global_binding->set_count(1);

  // Diffuse, emissive, normal, and specular maps.
  lava::descriptor::binding::ptr textures_binding =
      lava::make_descriptor_binding(1);
  textures_binding->set_type(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  textures_binding->set_stage_flags(VK_SHADER_STAGE_FRAGMENT_BIT);
  textures_binding->set_count(4);

  // Model matrix.
  lava::descriptor::binding::ptr object_binding =
      lava::make_descriptor_binding(2);
  object_binding->set_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  object_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  object_binding->set_count(1);

  descriptor_layout->add(global_binding);
  descriptor_layout->add(textures_binding);
  descriptor_layout->add(object_binding);
  descriptor_layout->create(app.device);
  return descriptor_layout;
}

fn create_bone_descriptors_layout(lava::app& app)
    ->std::tuple<lava::descriptor::ptr, lava::descriptor::ptr> {
  lava::descriptor::ptr descriptor_layout_global = lava::make_descriptor();
  lava::descriptor::ptr descriptor_layout_object = lava::make_descriptor();

  // Camera position float3 and View-Proj matrix
  lava::descriptor::binding::ptr global_binding =
      lava::make_descriptor_binding(0);
  global_binding->set_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  global_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  global_binding->set_count(1);

  // Model matrix, inverse bind poses, global transform poses, bone weights.
  lava::descriptor::binding::ptr model_binding =
      lava::make_descriptor_binding(0);
  model_binding->set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  model_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  model_binding->set_count(1);

  lava::descriptor::binding::ptr invbind_binding =
      lava::make_descriptor_binding(1);
  invbind_binding->set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  invbind_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  invbind_binding->set_count(1);

  lava::descriptor::binding::ptr globtrans_binding =
      lava::make_descriptor_binding(2);
  globtrans_binding->set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  globtrans_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  globtrans_binding->set_count(1);

  lava::descriptor::binding::ptr weights_binding =
      lava::make_descriptor_binding(3);
  weights_binding->set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  weights_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  weights_binding->set_count(1);

  descriptor_layout_global->add(global_binding);
  descriptor_layout_global->create(app.device);

  descriptor_layout_object->add(model_binding);
  descriptor_layout_object->add(invbind_binding);
  descriptor_layout_object->add(globtrans_binding);
  descriptor_layout_object->add(weights_binding);
  descriptor_layout_object->create(app.device);

  return {descriptor_layout_global, descriptor_layout_object};
}

fn create_graphics_pipeline(
    lava::app& app, lava::pipeline_layout::ptr pipeline_layout,
    std::vector<std::tuple<std::string, VkShaderStageFlagBits>>& shader_modules,
    VkPrimitiveTopology topology)
    ->lava::graphics_pipeline::ptr {
  lava::graphics_pipeline::ptr pipeline = make_graphics_pipeline(app.device);
  pipeline->set_layout(pipeline_layout);
  for (auto& shader : shader_modules) {
    pipeline->add_shader(lava::file_data(std::get<0>(shader)),
                         std::get<1>(shader));
  }
  pipeline->set_input_topology(topology);
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
      {3, 0, VK_FORMAT_R32G32_SFLOAT,
       lava::to_ui32(offsetof(lava::vertex, normal))},
  });
  lava::render_pass::ptr render_pass = app.shading.get_pass();
  success(pipeline->create(render_pass->get()), "FAIL pipeline create");
  render_pass->add_front(pipeline);
  return pipeline;
}
