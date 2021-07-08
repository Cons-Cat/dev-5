#version 450 core

layout(binding = 2) uniform sampler2D diffuse_color;

layout(binding = 3) uniform Ubo_Camera {
    mat4 projection;
    mat4 view;
}
ubo_camera;

layout(location = 0) in vec4 in_col;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = in_col * texture(diffuse_color, in_uv).rgba;
}
