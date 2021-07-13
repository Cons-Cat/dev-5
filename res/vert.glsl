#version 450 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_col;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_norm;

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
layout(location = 2) out vec3 out_pos;
layout(location = 3) out vec3 out_norm;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 vert_pos_inv = vec4(in_pos.x, in_pos.y * -1, in_pos.z, 1.0);
    gl_Position = ubo_camera.projection  * ubo_camera.view
        * ubo_model.model * vert_pos_inv;
    vec4 vert_pos_4 = // ubo_camera.view *
        ubo_model.model * vert_pos_inv;
    // vert_pos_4 *= ubo_camera.projection;
    out_pos = vec3(vert_pos_4);
    // out_pos.x = -dot(ubo_camera.view[0].xyz, ubo_camera.view[3].xyz);
    // out_pos.y = -dot(ubo_camera.view[1].xyz, ubo_camera.view[3].xyz);
    // out_pos.z = -dot(ubo_camera.view[2].xyz, ubo_camera.view[3].xyz);
    out_col = in_col;
    out_uv = in_uv;
    out_norm = in_norm;
}
