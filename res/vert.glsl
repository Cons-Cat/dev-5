#version 450 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_col;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_norm;

layout(binding = 0) uniform Ubo_Global {
    mat4 view_proj;
    vec3 pos;
}
ubo_camera;

layout(binding = 2) uniform Ubo_Object {
    mat4 model;
}
ubo_obj;

layout(location = 0) out vec4 out_col;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec4 out_pos_vert;
layout(location = 3) out vec4 out_pos_view;
layout(location = 4) out vec3 out_norm;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 vert_pos_inv = ubo_obj.model *
        vec4(in_pos.x, -in_pos.y, in_pos.z, 1.0);
    gl_Position = ubo_camera.view_proj * vert_pos_inv;

    out_pos_vert = vert_pos_inv;
    out_pos_view = vec4(ubo_camera.pos, 1);
    out_col = in_col;
    out_uv = in_uv;
    out_norm=vec3(1,1,1);
    // out_norm = vec3(in_norm.x, -in_norm.y, in_norm.z) * inverse(transpose(mat3(ubo_model.world)));
}
