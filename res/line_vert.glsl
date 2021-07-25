#version 450 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_col;

layout(binding = 0) uniform Ubo_Camera {
    mat4 view_proj;
    vec3 pos;
}
ubo_camera;

layout(binding = 2) readonly buffer Ubo_Object {
    mat4 model;
    mat4 inverse_bind[];
    mat4 global_transform[];
    float weight[];
}
ubo_object;

layout(location = 0) out vec4 out_col;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    out_col = in_col;
    gl_Position = ubo_camera.view_proj
        * ubo_object.model
        * vec4(in_pos.x, in_pos.y * -1, in_pos.z, 1.0);
}
