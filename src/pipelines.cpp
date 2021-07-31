#include "pipelines.h"

#include <iostream>

fn create_mesh_descriptor_layout(lava::app& app)
    ->std::tuple<lava::descriptor::ptr, lava::descriptor::ptr,
                 lava::descriptor::ptr, lava::descriptor::ptr> {
  lava::descriptor::ptr descriptor_layout_global = lava::make_descriptor();
  lava::descriptor::ptr descriptor_layout_textures = lava::make_descriptor();
  lava::descriptor::ptr descriptor_layout_object = lava::make_descriptor();
  lava::descriptor::ptr descriptor_layout_animation = lava::make_descriptor();

  // Camera position float3 and View-Proj matrix
  lava::descriptor::binding::ptr global_binding =
      lava::make_descriptor_binding(0);
  global_binding->set_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  global_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT |
                                  VK_SHADER_STAGE_FRAGMENT_BIT);
  global_binding->set_count(1);

  // Diffuse, emissive, normal, and specular maps.
  lava::descriptor::binding::ptr textures_binding =
      lava::make_descriptor_binding(0);
  textures_binding->set_type(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  textures_binding->set_stage_flags(VK_SHADER_STAGE_FRAGMENT_BIT);
  textures_binding->set_count(4);

  // Model matrix.
  lava::descriptor::binding::ptr object_binding =
      lava::make_descriptor_binding(0);
  object_binding->set_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  object_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  object_binding->set_count(1);

  // Animation data.
  lava::descriptor::binding::ptr invbind_binding =
      lava::make_descriptor_binding(0);
  invbind_binding->set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  invbind_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  invbind_binding->set_count(1);

  lava::descriptor::binding::ptr keyframe_trans_cur_binding =
      lava::make_descriptor_binding(1);
  keyframe_trans_cur_binding->set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  keyframe_trans_cur_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  keyframe_trans_cur_binding->set_count(1);

  lava::descriptor::binding::ptr keyframe_trans_next_binding =
      lava::make_descriptor_binding(2);
  keyframe_trans_next_binding->set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  keyframe_trans_next_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  keyframe_trans_next_binding->set_count(1);

  lava::descriptor::binding::ptr keyframe_binding =
      lava::make_descriptor_binding(3);
  keyframe_binding->set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  keyframe_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  keyframe_binding->set_count(1);

  descriptor_layout_global->add(global_binding);
  descriptor_layout_textures->add(textures_binding);
  descriptor_layout_object->add(object_binding);
  descriptor_layout_animation->add(invbind_binding);
  descriptor_layout_animation->add(keyframe_trans_cur_binding);
  descriptor_layout_animation->add(keyframe_trans_next_binding);
  descriptor_layout_animation->add(keyframe_binding);

  descriptor_layout_global->create(app.device);
  descriptor_layout_textures->create(app.device);
  descriptor_layout_object->create(app.device);
  descriptor_layout_animation->create(app.device);
  return {descriptor_layout_global, descriptor_layout_textures,
          descriptor_layout_object, descriptor_layout_animation};
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

  // Model matrix, inverse bind poses, global transform poses, bone weights, and
  // keyframe.
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

  lava::descriptor::binding::ptr keyframe_trans_cur_binding =
      lava::make_descriptor_binding(2);
  keyframe_trans_cur_binding->set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  keyframe_trans_cur_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  keyframe_trans_cur_binding->set_count(1);

  lava::descriptor::binding::ptr keyframe_trans_next_binding =
      lava::make_descriptor_binding(3);
  keyframe_trans_next_binding->set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  keyframe_trans_next_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  keyframe_trans_next_binding->set_count(1);

  lava::descriptor::binding::ptr keyframe_binding =
      lava::make_descriptor_binding(4);
  keyframe_binding->set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  keyframe_binding->set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT);
  keyframe_binding->set_count(1);

  descriptor_layout_global->add(global_binding);
  descriptor_layout_global->create(app.device);

  descriptor_layout_object->add(model_binding);
  descriptor_layout_object->add(invbind_binding);
  descriptor_layout_object->add(keyframe_trans_cur_binding);
  descriptor_layout_object->add(keyframe_trans_next_binding);
  descriptor_layout_object->add(keyframe_binding);
  descriptor_layout_object->create(app.device);
  return {descriptor_layout_global, descriptor_layout_object};
}
