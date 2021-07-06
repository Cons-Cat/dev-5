glslc --target-env=vulkan -x glsl -finvert-y -fshader-stage=vertex -o vert.spv vert.glsl 
glslc --target-env=vulkan -x glsl -fshader-stage=fragment -o frag.spv frag.glsl

glslc --target-env=vulkan -x glsl -finvert-y -fshader-stage=vertex -o line_vert.spv line_vert.glsl 
glslc --target-env=vulkan -x glsl -fshader-stage=fragment -o line_frag.spv line_frag.glsl
