#version 450 core

layout(binding = 2) uniform Ubo_Camera {
    mat4 projection;
    mat4 view;
}
ubo_camera;

layout(binding = 3) uniform sampler2D diffuse_color;
layout(binding = 4) uniform sampler2D emissive_color;
layout(binding = 5) uniform sampler2D normal_color;
layout(binding = 6) uniform sampler2D specular_color;

layout(location = 0) in vec4 in_col;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_norm;

layout(location = 0) out vec4 out_color;

// Light source
vec3 light_direction = vec3(1, 1, 1);

void main() {
    float brightness = max(dot(in_norm, normalize(light_direction)), 0.5);
    out_color = vec4(texture(diffuse_color, in_uv).rgb * brightness, 1);
}
