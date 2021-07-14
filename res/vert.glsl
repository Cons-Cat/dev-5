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
    mat4 world;
}
ubo_model;

layout(binding = 2) uniform Ubo_Camera_Pos {
    vec3 cam_pos;
}
ubo_cam_pos;

layout(location = 0) out vec4 out_col;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec4 out_pos_vert;
layout(location = 3) out vec4 out_pos_view;
layout(location = 4) out vec3 out_norm;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 vert_pos_inv = ubo_model.world * vec4(in_pos.x, -in_pos.y, in_pos.z, 1.0);
    gl_Position = ubo_camera.projection * ubo_camera.view
        * vert_pos_inv;
    out_pos_vert = vert_pos_inv;

    out_pos_view = vec4(ubo_cam_pos.cam_pos, 1);
    out_col = in_col;
    out_uv = in_uv;
    out_norm = vec3(in_norm.x, -in_norm.y, in_norm.z) * inverse(transpose(mat3(ubo_model.world)));
}
