#version 450 core

struct Transform {
    vec3 translation;
    // From GLM: x, y, z, w
    vec4 quaternion;
};

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_col;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_norm;
layout(location = 4) in uvec4 in_weight_indices;
layout(location = 5) in vec4 in_bone_weights;

layout(set = 0, binding = 0) uniform Ubo_Global {
    mat4 view_proj;
    vec3 pos;
}
ubo_camera;

layout(set = 2, binding = 0) uniform Ubo_Object {
    mat4 model;
}
ubo_obj;

// TODO: Rename Object to Animation
layout(set = 3, binding = 0) readonly buffer Ssbo_Object_InvBind {
    mat4 inverse_bind[];
};
layout(set = 3, binding = 1) readonly buffer Ssbo_Object_KeyframeTrans_Cur {
    Transform global_keyframetrans_cur[];
};
layout(set = 3, binding = 2) readonly buffer Ssbo_Object_KeyframeTrans_Next {
    Transform global_keyframetrans_next[];
};
layout(set = 3, binding = 3) readonly buffer Ssbo_Object_Keyframe {
    float keyframe;
};

layout(location = 0) out vec4 out_col;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec4 out_pos_vert;
layout(location = 3) out vec4 out_pos_view;
layout(location = 4) out vec3 out_norm;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    int idx = gl_VertexIndex;

    mat4 inverse_bind_cur;
    for (int i = 0; i < inverse_bind.length(); i++) {
        if (i == idx) {
            inverse_bind_cur = inverse_bind[i];
            break;
        }
    }

    vec4 vert_pos_inv = 1 *
        inverse_bind_cur *
        ubo_obj.model *
        vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);

    out_pos_vert = vert_pos_inv;
    out_pos_view = vec4(ubo_camera.pos, 1);
    out_col = in_col;
    out_uv = in_uv;
    out_norm = vec3(in_norm.x, -in_norm.y, in_norm.z)
        * inverse(transpose(mat3(ubo_obj.model)));

    gl_Position = 1
        * ubo_camera.view_proj
        * vec4(vert_pos_inv.x, -vert_pos_inv.y, vert_pos_inv.z, 1.0);
}
