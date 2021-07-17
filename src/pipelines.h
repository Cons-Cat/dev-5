#pragma once

#include <liblava/lava.hpp>

#include "includes.h"

fn create_mesh_descriptor_layout(lava::app& app)->lava::descriptor::ptr;

fn create_graphics_pipeline(
    lava::app& app, lava::pipeline_layout::ptr pipeline_layout,
    lava::descriptor::ptr descriptor_layout,
    std::vector<std::tuple<std::string, VkShaderStageFlagBits>>&
        shader_modules)
    ->lava::graphics_pipeline::ptr;
