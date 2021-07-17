#include "pipelines.h"

fn create_mesh_descriptor_layout(lava::app app)->lava::descriptor::ptr {
  lava::descriptor::ptr descriptor_layout = lava::make_descriptor();
  descriptor_layout->add_binding(
      0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      VK_SHADER_STAGE_VERTEX_BIT);  // MVP matrix and camera position vector.
  descriptor_layout->add_binding(
      1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      VK_SHADER_STAGE_FRAGMENT_BIT);  // Diffuse, emissive, normal, and specular
                                      // texture maps.
  descriptor_layout->create(app.device);
  return descriptor_layout;
}

fn build_pipeline(lava::app::ptr app,
                  lava::pipeline_layout::ptr pipeline_layout,
                  lava::descriptor& descriptor_layout,
                  std::vector<lava::unique_data>& shader_modules)
    ->lava::graphics_pipeline::ptr {
  return nullptr;
}
