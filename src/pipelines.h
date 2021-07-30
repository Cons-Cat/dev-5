#pragma once

#include <liblava/lava.hpp>

#include "includes.h"

fn create_mesh_descriptor_layout(lava::app& app)->lava::descriptor::ptr;

fn create_bone_descriptors_layout(lava::app& app)
    ->std::tuple<lava::descriptor::ptr, lava::descriptor::ptr>;

template <typename T>
fn create_graphics_pipeline(
    lava::app& app, lava::pipeline_layout::ptr pipeline_layout,
    std::vector<std::tuple<std::string, VkShaderStageFlagBits>>& shader_modules,
    lava::VkVertexInputAttributeDescriptions vertex_attributes,
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    ->lava::graphics_pipeline::ptr;

#include "pipelines.tpp"
