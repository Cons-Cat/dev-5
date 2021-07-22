#include <fbxsdk.h>
#include <imgui.h>

#include <cstddef>
#include <iostream>
#include <liblava/lava.hpp>
#include <typeinfo>

#include "fbx_loading.h"
#include "includes.h"
#include "pipelines.h"

using fbxsdk::FbxNode;

enum RenderMode { mesh, skeleton };
static RenderMode render_mode;  // Initialized in main()
static std::vector<AnimationClip> anim_clips;

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
  lava::mesh_template_data loaded_data = find_fbx_mesh(root_node).value();
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
  app.config.surface.formats = {VK_FORMAT_B8G8R8A8_SRGB};
  app.camera.rotation_speed = 250;
  app.camera.movement_speed += 10;
  success(app.setup(), "Failed to setup app.");

  app.camera.position = lava::v3(0.0f, -4.036f, 8.304f);
  app.camera.rotation = lava::v3(-15, 0, 0);
  lava::mat4 mesh_model_mat = lava::mat4(1.0);  // This is an identity matrix.

  // Bones
  lava::buffer bones_buffer;
  lava::mesh_data bone_mesh_data;

  for (size_t i = 0; i < joints.size(); i++) {
    Joint *cur_joint = joints[i];
    Joint *par_joint = joints[cur_joint->parent_index];
    FbxVector4 cur_origin = joints[i]->transform.GetRow(3);
    FbxVector4 par_origin = par_joint->transform.GetRow(3);
    auto diff = par_origin - cur_origin;
    auto cur_mat = glm::identity<glm::dmat4x4>();
    auto cur_vec = cur_origin + diff;
    // FBX Matrices are column-major double-floating precision.
    cur_mat[3] =
        static_cast<glm::vec4>(*reinterpret_cast<glm::dvec4 *>(&cur_vec));

    bone_mesh_data.vertices.push_back(
        lava::vertex{.position = fbxvec_to_glmvec(cur_origin)});
    bone_mesh_data.vertices.push_back(
        lava::vertex{.position = fbxvec_to_glmvec(par_origin)});
  }

  // bones_buffer.create_mapped(app.device, &bone_mesh_data,
  //                            sizeof(lava::vertex) * 2 * joints.size(),
  //                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

  lava::mesh::ptr bones_mesh = lava::make_mesh();
  bones_mesh->add_data(bone_mesh_data);
  bones_mesh->create(app.device);

  // Load keyframe.
  FbxAnimStack *anim_stack = scene->GetCurrentAnimationStack();
  FbxTimeSpan time_span = anim_stack->GetLocalTimeSpan();
  FbxTime real_time = time_span.GetDuration();
  AnimationClip anim_clip{};
  auto fps = FbxTime::EMode::eFrames24;
  anim_clip.duration = real_time.GetFrameCount(fps);

  for (double i = 1; i < anim_clip.duration; i++) {
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

  // Load textures
  // TODO: Abstract as function
  lava::texture::ptr diffuse_texture =
      lava::load_texture(app.device, "../res/Idle.fbm/PPG_3D_Player_D.png");
  lava::texture::ptr emissive_texture = lava::load_texture(
      app.device, "../res/Idle.fbm/PPG_3D_Player_emissive.png");
  lava::texture::ptr normal_texture =
      lava::load_texture(app.device, "../res/Idle.fbm/PPG_3D_Player_N.png");
  lava::texture::ptr specular_texture =
      lava::load_texture(app.device, "../res/Idle.fbm/PPG_3D_Player_spec.png");
  app.staging.add(diffuse_texture);
  app.staging.add(emissive_texture);
  app.staging.add(normal_texture);
  app.staging.add(specular_texture);
  std::array<VkDescriptorImageInfo, 4> textures_descriptor_info{{
      *diffuse_texture->get_descriptor_info(),
      *emissive_texture->get_descriptor_info(),
      *normal_texture->get_descriptor_info(),
      *specular_texture->get_descriptor_info(),
  }};

  typedef struct {
    lava::mat4 view_proj;
    alignas(16) lava::v3 cam_pos;
  } CameraBuffer;

  CameraBuffer camera_buffer_data = {lava::mat4(1), app.camera.position};

  lava::buffer camera_buffer;
  camera_buffer.create_mapped(
      app.device, &camera_buffer_data, sizeof(camera_buffer_data),
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

  // Load mesh.
  lava::mesh::ptr made_mesh = lava::make_mesh();
  made_mesh->add_data(loaded_data);
  lava::buffer object_buffer;
  object_buffer.create_mapped(app.device, &mesh_model_mat,
                              sizeof(mesh_model_mat),
                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  made_mesh->create(app.device);

  lava::graphics_pipeline::ptr mesh_pipeline;
  lava::descriptor::ptr mesh_descriptor_layout;
  lava::pipeline_layout::ptr mesh_pipeline_layout;
  VkDescriptorSet mesh_descriptor_set = VK_NULL_HANDLE;

  lava::graphics_pipeline::ptr bone_pipeline;
  lava::descriptor::ptr bone_descriptor_layout;
  lava::pipeline_layout::ptr bone_pipeline_layout;
  VkDescriptorSet bone_descriptor_set = VK_NULL_HANDLE;

  lava::descriptor::pool::ptr descriptor_pool;
  descriptor_pool = lava::make_descriptor_pool();
  descriptor_pool->create(app.device,
                          {
                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4},
                              {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
                          },
                          2);

  app.on_create = [&]() {
    std::cout
        << app.device->get_properties().limits.minUniformBufferOffsetAlignment
        << '\n';

    // TODO: Push descriptors to this, then update all here.
    // std::vector<VkWriteDescriptorSet> descriptor_writes;
    mesh_descriptor_layout = create_mesh_descriptor_layout(app);
    mesh_pipeline_layout = lava::make_pipeline_layout();
    mesh_pipeline_layout->add(mesh_descriptor_layout);
    mesh_pipeline_layout->create(app.device);
    mesh_descriptor_set =
        mesh_descriptor_layout->allocate(descriptor_pool->get());

    bone_descriptor_layout = create_bone_descriptor_layout(app);
    bone_pipeline_layout = lava::make_pipeline_layout();
    bone_pipeline_layout->add(bone_descriptor_layout);
    bone_pipeline_layout->create(app.device);
    bone_descriptor_set =
        bone_descriptor_layout->allocate(descriptor_pool->get());

    // TODO: Move descriptor writes into a new func that also does descriptor
    // layout allocation.
    {
      VkWriteDescriptorSet const descriptor_global{
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = mesh_descriptor_set,
          .dstBinding = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pBufferInfo = camera_buffer.get_descriptor_info(),
      };
      VkWriteDescriptorSet const descriptor_textures{
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = mesh_descriptor_set,
          .dstBinding = 1,
          .descriptorCount = 4,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .pImageInfo = &textures_descriptor_info.front(),
      };
      VkWriteDescriptorSet const descriptor_object{
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = mesh_descriptor_set,
          .dstBinding = 2,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pBufferInfo = object_buffer.get_descriptor_info(),
      };

      VkWriteDescriptorSet const descriptor_global_bone{
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = bone_descriptor_set,
          .dstBinding = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pBufferInfo = camera_buffer.get_descriptor_info(),
      };

      app.device->vkUpdateDescriptorSets(
          {descriptor_global, descriptor_global_bone, descriptor_textures,
           descriptor_object});
    }

    mesh_pipeline_layout = lava::make_pipeline_layout();
    mesh_pipeline_layout->add(mesh_descriptor_layout);
    mesh_pipeline_layout->create(app.device);

    bone_pipeline_layout = lava::make_pipeline_layout();
    bone_pipeline_layout->add(bone_descriptor_layout);
    bone_pipeline_layout->create(app.device);

    {
      using shader_module_t = std::tuple<std::string, VkShaderStageFlagBits>;
      auto shader_modules = std::vector<shader_module_t>();
      shader_modules.push_back(
          shader_module_t("../res/vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
      shader_modules.push_back(
          shader_module_t("../res/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));
      mesh_pipeline = create_graphics_pipeline(
          app, mesh_pipeline_layout, mesh_descriptor_layout, shader_modules);
    }

    {
      // TODO: Better line shaders
      using shader_module_t = std::tuple<std::string, VkShaderStageFlagBits>;
      auto shader_modules = std::vector<shader_module_t>();
      shader_modules.push_back(
          shader_module_t("../res/line_vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
      shader_modules.push_back(shader_module_t("../res/line_frag.spv",
                                               VK_SHADER_STAGE_FRAGMENT_BIT));
      bone_pipeline = create_graphics_pipeline(
          app, bone_pipeline_layout, bone_descriptor_layout, shader_modules,
          VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    }

    // Default to rendering the mesh.
    render_mode = mesh;
    return true;
  };

  app.imgui.on_draw = [&]() {
    ImGui::SetNextWindowPos({30, 30}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({330, 485}, ImGuiCond_FirstUseEver);
    ImGui::Begin(app.get_name());
    ImGui::SameLine(0.f, 15.f);
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::SameLine(0.f, 15.f);
    bool camera_active = app.camera.activated();
    if (ImGui::Checkbox("active", &camera_active))
      app.camera.set_active(camera_active);
    ImGui::SameLine(0.f, 10.f);
    bool first_person = app.camera.mode == lava::camera_mode::first_person;
    if (ImGui::Checkbox("first person##camera", &first_person))
      app.camera.mode = first_person ? lava::camera_mode::first_person
                                     : lava::camera_mode::look_at;
    ImGui::Spacing();
    ImGui::DragFloat3("position##camera", (lava::r32 *)&app.camera.position,
                      0.01f);
    ImGui::DragFloat3("rotation##camera", (lava::r32 *)&app.camera.rotation,
                      0.1f);
    ImGui::Spacing();
    ImGui::Checkbox("lock rotation##camera", &app.camera.lock_rotation);
    ImGui::SameLine(0.f, 10.f);
    ImGui::Checkbox("lock z##camera", &app.camera.lock_z);
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("speed")) {
      ImGui::DragFloat("movement##camera", &app.camera.movement_speed, 0.1f);
      ImGui::DragFloat("rotation##camera", &app.camera.rotation_speed, 0.1f);
      ImGui::DragFloat("zoom##camera", &app.camera.zoom_speed, 0.1f);
    }
    if (ImGui::CollapsingHeader("projection")) {
      bool update_projection = false;
      update_projection |= ImGui::DragFloat("fov", &app.camera.fov);
      update_projection |= ImGui::DragFloat("z near", &app.camera.z_near);
      update_projection |= ImGui::DragFloat("z far", &app.camera.z_far);
      update_projection |= ImGui::DragFloat("aspect", &app.camera.aspect_ratio);
      if (update_projection) app.camera.update_projection();
    }
    app.draw_about();
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("enter = first person\nr = lock rotation\nz = lock z");
    ImGui::End();
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
    app.camera.update_view(dt, app.input.get_mouse_position());
    app.camera.update_projection();
    mesh_pipeline->on_process = nullptr;
    bone_pipeline->on_process = nullptr;

    camera_buffer_data.view_proj = app.camera.get_view_projection();
    camera_buffer_data.cam_pos = app.camera.position;

    if (render_mode == mesh) {
      memcpy(object_buffer.get_mapped_data(), &mesh_model_mat,
             sizeof(mesh_model_mat));
      memcpy(camera_buffer.get_mapped_data(), &camera_buffer_data,
             sizeof(camera_buffer_data));

      mesh_pipeline->on_process = [&](VkCommandBuffer cmd_buf) {
        mesh_pipeline_layout->bind(cmd_buf, mesh_descriptor_set);
        made_mesh->bind_draw(cmd_buf);
      };
    } else if (render_mode == skeleton) {
      bone_pipeline->on_process = [&](VkCommandBuffer cmd_buf) {
        bone_pipeline_layout->bind(cmd_buf, bone_descriptor_set);
        bones_mesh->bind_draw(cmd_buf);
      };
    }
    app.camera.update_view(dt, app.input.get_mouse_position());
    return true;
  };

  return app.run();
}
