#version 450 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_col;
layout(location = 2) in vec2 in_uv;

layout(binding = 0) uniform Ubo_Camera {
    mat4 projection;
    mat4 view;
}
ubo_camera;

layout(binding = 1) uniform Ubo_Model {
    mat4 model;
}
ubo_model;

layout(location = 0) out vec4 out_col;
layout(location = 1) out vec2 out_uv;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    out_col = in_col;
    out_uv = in_uv;
    gl_Position = ubo_camera.projection  * ubo_camera.view
        * ubo_model.model * vec4(in_pos.x, -in_pos.y, in_pos.z, 1.0);
}
