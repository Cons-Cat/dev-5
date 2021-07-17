#pragma once

#include <liblava/lava.hpp>

#include "includes.h"

fn create_mesh_descriptor_layout(lava::app app)->lava::descriptor::ptr;

fn create_pipeline(lava::app::ptr app,
                   lava::pipeline_layout::ptr pipeline_layout,
                   lava::descriptor& descriptor_layout,
                   std::vector<lava::unique_data>& shader_modules)
    ->lava::graphics_pipeline::ptr;
