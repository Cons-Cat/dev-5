#include "fbx_loading.h"
#include "includes.h"
#include <cstddef>
#include <fbxsdk.h>
#include <iostream>
#include <liblava/lava.hpp>
#include <typeinfo>

using fbxsdk::FbxNode;

enum RenderMode { mesh, skeleton };
static RenderMode render_mode; // Initialized in main()
static std::vector<AnimationClip> anim_clips;

VkWriteDescriptorSet write_desc_ubo_camera_mesh;
VkWriteDescriptorSet write_desc_ubo_model_mesh;
VkWriteDescriptorSet write_desc_sampler_mesh;
VkWriteDescriptorSet write_desc_ubo_camera_bone;
VkWriteDescriptorSet write_desc_ubo_model_bone;

fn make_mesh_pipeline(lava::app &app, lava::graphics_pipeline::ptr &pipeline,
                      lava::descriptor::ptr &descriptor_layout,
                      lava::descriptor::pool::ptr &descriptor_pool,
                      lava::pipeline_layout::ptr &pipeline_layout,
                      VkDescriptorSet &descriptor_set,
                      lava::buffer &model_buffer,
                      lava::texture::ptr &loaded_texture) {
  pipeline = make_graphics_pipeline(app.device);
  success((pipeline->add_shader(lava::file_data("../res/vert.spv"),
                                VK_SHADER_STAGE_VERTEX_BIT)),
          "Failed to load vertex shader.");
  success((pipeline->add_shader(lava::file_data("../res/frag.spv"),
                                VK_SHADER_STAGE_FRAGMENT_BIT)),
          "Failed to load fragment shader.");
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
      {3, 0, VK_FORMAT_R32G32B32_SFLOAT,
       lava::to_ui32(offsetof(lava::vertex, normal))},
  });
  descriptor_layout = lava::make_descriptor();
  descriptor_layout->add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT);
  descriptor_layout->add_binding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT);
  descriptor_layout->add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                 VK_SHADER_STAGE_FRAGMENT_BIT);
  success((descriptor_layout->create(app.device)),
          "Failed to create descriptor layout.");
  success((descriptor_pool->create(
              app.device, {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},
                           {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}})),
          "Failed to create descriptor pool.");
  pipeline_layout = lava::make_pipeline_layout();
  pipeline_layout->add(descriptor_layout);
  success((pipeline_layout->create(app.device)),
          "Failed to create pipeline layout.");
  pipeline->set_layout(pipeline_layout);
  descriptor_set = descriptor_layout->allocate(descriptor_pool->get());
  write_desc_ubo_camera_mesh = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_set,
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo = app.camera.get_descriptor_info(),
  };
  write_desc_ubo_model_mesh = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_set,
      .dstBinding = 1,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo = model_buffer.get_descriptor_info(),
  };
  write_desc_sampler_mesh = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_set,
      .dstBinding = 2,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = loaded_texture->get_descriptor_info(),
  };
  app.device->vkUpdateDescriptorSets({write_desc_ubo_camera_mesh,
                                      write_desc_ubo_model_mesh,
                                      write_desc_sampler_mesh});
  lava::render_pass::ptr render_pass = app.shading.get_pass();
  success((pipeline->create(render_pass->get())), "Failed to make pipeline.");
  render_pass->add_front(pipeline);
}

fn make_bone_pipeline(lava::app &app, lava::graphics_pipeline::ptr &pipeline,
                      lava::descriptor::ptr &descriptor_layout,
                      lava::descriptor::pool::ptr &descriptor_pool,
                      lava::pipeline_layout::ptr &pipeline_layout,
                      VkDescriptorSet &descriptor_set,
                      lava::buffer &model_buffer) {
  pipeline = make_graphics_pipeline(app.device);
  success((pipeline->add_shader(lava::file_data("../res/line_vert.spv"),
                                VK_SHADER_STAGE_VERTEX_BIT)),
          "Failed to load vertex shader.");
  success((pipeline->add_shader(lava::file_data("../res/line_frag.spv"),
                                VK_SHADER_STAGE_FRAGMENT_BIT)),
          "Failed to load fragment shader.");
  pipeline->add_color_blend_attachment();
  pipeline->set_depth_test_and_write();
  pipeline->set_depth_compare_op(VK_COMPARE_OP_LESS_OR_EQUAL);
  pipeline->set_vertex_input_binding(
      {0, sizeof(lava::vertex), VK_VERTEX_INPUT_RATE_VERTEX});
  pipeline->set_vertex_input_attributes({
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT,
       lava::to_ui32(offsetof(lava::vertex, position))},
      // {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
      //  lava::to_ui32(offsetof(lava::vertex, color))},
      // {2, 0, VK_FORMAT_R32G32_SFLOAT,
      //  lava::to_ui32(offsetof(lava::vertex, uv))},
      // {3, 0, VK_FORMAT_R32G32B32_SFLOAT,
      //  lava::to_ui32(offsetof(lava::vertex, normal))},
  });
  descriptor_layout = lava::make_descriptor();
  descriptor_layout->add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT);
  descriptor_layout->add_binding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 VK_SHADER_STAGE_VERTEX_BIT);
  success((descriptor_layout->create(app.device)),
          "Failed to create descriptor layout.");
  success((descriptor_pool->create(app.device,
                                   {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2}})),
          "Failed to create descriptor pool.");
  pipeline_layout = lava::make_pipeline_layout();
  pipeline_layout->add(descriptor_layout);
  success((pipeline_layout->create(app.device)),
          "Failed to create pipeline layout.");
  pipeline->set_layout(pipeline_layout);
  descriptor_set = descriptor_layout->allocate(descriptor_pool->get());
  write_desc_ubo_camera_bone = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_set,
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo = app.camera.get_descriptor_info(),
  };
  write_desc_ubo_model_bone = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_set,
      .dstBinding = 1,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo = model_buffer.get_descriptor_info(),
  };
  app.device->vkUpdateDescriptorSets(
      {write_desc_ubo_camera_bone, write_desc_ubo_model_bone});
  lava::render_pass::ptr render_pass = app.shading.get_pass();
  success((pipeline->create(render_pass->get())), "Failed to make pipeline.");
  render_pass->add_front(pipeline);
}

int main(int argc, char *argv[]) {
  // Load and read the mesh from an FBX.
  // std::string path = "../res/Teddy/Teddy_Idle.fbx";
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
  lava::mesh_data loaded_data = find_fbx_mesh(root_node).value();
  // manager->Destroy();
  std::cout << "Path: " << path << std::endl;

  // Load the skeleton.
  // Keyframe keyframe;
  std::vector<FbxPose *> poses;
  // Fill out poses.
  find_fbx_poses(root_node, &poses);
  FbxPose *bind_pose = nullptr;
  std::cout << "Poses: " << poses.size() << std::endl;
  for (size_t i = 0; i < poses.size(); i++) {
    if (poses[i]->IsBindPose()) {
      std::cout << "Found a bind pose.\n";
      bind_pose = poses[i];
      break;
    }
  }
  auto pose_count = scene->GetPoseCount();
  for (size_t i = 0; i < pose_count; i++) {
    auto cur_pose = scene->GetPose(i);
    if (cur_pose->IsBindPose()) {
      bind_pose = cur_pose;
      break;
    }
  }
  success(bind_pose, "Failed to find a bind pose.\n");

  // Find skeleton.
  FbxSkeleton *root_skel = nullptr;
  for (size_t i = 0; i < bind_pose->GetCount(); i++) {
    auto cur_skel = bind_pose->GetNode(i)->GetSkeleton();
    if (cur_skel && cur_skel->IsSkeletonRoot()) {
      root_skel = cur_skel;
    }
  }
  success(root_skel, "Failed to find a root skeleton.");

  std::vector<Joint *> joints;
  auto make_joint = [&](FbxNode *node, int index) -> Joint * {
    // TODO: Factor into unique ptr.
    return new Joint{.node = node,
                     .parent_index = index,
                     .transform = node->EvaluateGlobalTransform()};
  };
  std::function<void(Joint *, int, size_t)> get_joints =
      [&](Joint *parent_joint, int parent_index, size_t parent_breadth) {
        size_t children = parent_joint->node->GetChildCount();
        for (size_t i = 0; i < children; i++) {
          auto cur_node = parent_joint->node->GetChild(i);
          if (cur_node && cur_node->GetNodeAttribute() &&
              cur_node->GetNodeAttribute()->GetAttributeType() ==
                  FbxNodeAttribute::eSkeleton) {
            auto new_joint = make_joint(cur_node, parent_index);
            joints.push_back(new_joint);
            get_joints(new_joint, joints.size() - 1, children);
          }
        }
      };
  get_joints(make_joint(root_skel->GetNode(), -1), 0, 0);

  // Render the mesh.
  lava::app app("DEV 5 - WGooch", {argc, argv});
  // app.manager.on_create_param = [](lava::device::create_param& param) {
    // app.config.req_api_version=lava::api_version::v1_2;
  // };
  app.config.surface.formats = {VK_FORMAT_B8G8R8A8_SRGB};
  success(app.setup(), "Failed to setup app.");
  lava::mesh::ptr made_mesh = lava::make_mesh();
  made_mesh->add_data(loaded_data);
  lava::texture::ptr loaded_texture =
      // create_default_texture(app.device, {4096, 4096});
      lava::load_texture(app.device, "../res/Idle.fbm/PPG_3D_Player_D.png");
  app.staging.add(loaded_texture);
  app.camera.position = lava::v3(0.0f, -4.036f, 8.304f);
  app.camera.rotation = lava::v3(-15, 0, 0);
  lava::mat4 model_space = lava::mat4(1.0); // This is an identity matrix.

  lava::descriptor::pool::ptr descriptor_pool;
  descriptor_pool = lava::make_descriptor_pool();

  // Load mesh.
  lava::buffer model_buffer;
  success(model_buffer.create_mapped(app.device, &model_space,
                                     sizeof(float) * 16,
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
          "Failed to map mesh buffer.");
  made_mesh->create(app.device);

  lava::graphics_pipeline::ptr mesh_pipeline;
  lava::descriptor::ptr mesh_descriptor_layout;
  lava::pipeline_layout::ptr mesh_pipeline_layout;
  VkDescriptorSet mesh_descriptor_set = VK_NULL_HANDLE;

  // Bones
  lava::buffer bones_buffer;
  auto bone_meshes = new lava::mesh::ptr[joints.size()];
  // Hard coded 30 should be larger than the number of bones.
  auto bones_data = new std::array<glm::mat4x4, 3>[joints.size()];
  for (size_t i = 0; i < joints.size(); i++) {
    Joint *cur_joint = joints[i];
    Joint *par_joint = joints[cur_joint->parent_index];
    FbxVector4 cur_origin = joints[i]->transform.GetRow(3);
    FbxVector4 par_origin = par_joint->transform.GetRow(3);
    auto diff = par_origin - cur_origin;
    auto norm = FbxVector4(-diff[1], diff[0], diff[2], 0) / 15;
    auto cur_mat_one = glm::identity<glm::dmat4x4>();
    auto cur_mat_two = glm::identity<glm::dmat4x4>();
    auto cur_mat_three = glm::identity<glm::dmat4x4>();
    auto cur_vec_one = (cur_origin + norm);
    auto cur_vec_two = cur_origin - norm;
    auto cur_vec_three = cur_origin + diff;
    // FBX Matrices are column-major double-floating precision.
    cur_mat_one[3] =
        static_cast<glm::vec4>(*reinterpret_cast<glm::dvec4 *>(&cur_vec_one));
    cur_mat_two[3] =
        static_cast<glm::vec4>(*reinterpret_cast<glm::dvec4 *>(&cur_vec_two));
    cur_mat_three[3] =
        static_cast<glm::vec4>(*reinterpret_cast<glm::dvec4 *>(&cur_vec_three));
    bones_data[i] =
        std::array<glm::mat4x4, 3>{cur_mat_one, cur_mat_two, cur_mat_three};
    lava::mesh_data cur_bone_mesh_data;
    cur_bone_mesh_data.vertices.push_back(
        lava::vertex{.position = fbxvec_to_glmvec(cur_vec_one)});
    cur_bone_mesh_data.vertices.push_back(
        lava::vertex{.position = fbxvec_to_glmvec(cur_vec_two)});
    cur_bone_mesh_data.vertices.push_back(
        lava::vertex{.position = fbxvec_to_glmvec(cur_vec_three)});
    bone_meshes[i] = lava::make_mesh();
    bone_meshes[i]->add_data(cur_bone_mesh_data);
    bone_meshes[i]->create(app.device);
  }
  success(bones_buffer.create_mapped(app.device, &bones_data,
                                     sizeof(glm::vec3) * 3 * joints.size(),
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
          "Failed to map bones buffer.");

  // Load keyframe.
  FbxAnimStack *anim_stack = scene->GetCurrentAnimationStack();
  FbxTimeSpan time_span = anim_stack->GetLocalTimeSpan();
  FbxTime real_time = time_span.GetDuration();
  AnimationClip anim_clip{};
  auto fps = FbxTime::EMode::eFrames24;
  anim_clip.duration = real_time.GetFrameCount(fps);
  for (double i = 1; i < anim_clip.duration; i++) {
    // std::cout << "Frame update." << std::endl;
    Keyframe cur_keyframe;
    real_time.SetFrame(i, fps);
    cur_keyframe.time = i;
    for (auto joint : joints) {
      cur_keyframe.joints.push_back(new Joint{
          .node = joint->node,
          .parent_index = joint->parent_index,
          .transform = joint->node->EvaluateGlobalTransform(real_time)});
    }
    anim_clip.frames.push_back(cur_keyframe);
  }

  // Put meshes in GPU.
  lava::graphics_pipeline::ptr bone_pipeline;
  // lava::descriptor::pool::ptr bone_descriptor_pool;
  lava::descriptor::ptr bone_descriptor_layout;
  lava::pipeline_layout::ptr bone_pipeline_layout;
  VkDescriptorSet bone_descriptor_set = VK_NULL_HANDLE;

  app.on_create = [&]() {
    make_mesh_pipeline(app, mesh_pipeline, mesh_descriptor_layout,
                       descriptor_pool, mesh_pipeline_layout,
                       mesh_descriptor_set, model_buffer, loaded_texture);

    make_bone_pipeline(app, bone_pipeline, bone_descriptor_layout,
                       descriptor_pool, bone_pipeline_layout,
                       bone_descriptor_set, model_buffer);

    // Start by rendering the mesh.
    render_mode = skeleton;
    // TODO: Move back into make_mesh_pipeline()

    return true;
  };

  app.input.key.listeners.add([&](lava::key_event::ref event) {
    if (event.pressed(lava::key::_1)) {
      render_mode = mesh;
      return true;
    } else if (event.pressed(lava::key::_2)) {
      render_mode = skeleton;
      return true;
    }
    return false;
  });

  app.on_update = [&](lava::delta dt) {
    // Command buffers
    mesh_pipeline->on_process = nullptr;
    bone_pipeline->on_process = nullptr;
    if (render_mode == mesh) {
      mesh_pipeline->on_process = [&](VkCommandBuffer cmd_buf) {
        mesh_pipeline_layout->bind(cmd_buf, mesh_descriptor_set);
        made_mesh->bind_draw(cmd_buf);
      };
    } else if (render_mode == skeleton) {
      bone_pipeline->on_process = [&](VkCommandBuffer cmd_buf) {
        bone_pipeline_layout->bind(cmd_buf, bone_descriptor_set);
        for (size_t i = 0; i < joints.size(); i++) {
          bone_meshes[i]->bind_draw(cmd_buf);
        }
      };
    }
    app.camera.update_view(dt, app.input.get_mouse_position());
    return true;
  };

  return app.run();
}
