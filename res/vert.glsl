#version 450 core

struct Transform {
    vec3 translation;
    // From GLM: x, y, z, w
    vec4 quaternion;
};

vec4 normalize_quaternion(vec4 in_quat) {
    vec4 out_quat;
    float norm = sqrt(in_quat.x * in_quat.x + in_quat.y * in_quat.y +
                      in_quat.z * in_quat.z + in_quat.w * in_quat.w);
    out_quat.x = in_quat.x / norm;
    out_quat.y = in_quat.y / norm;
    out_quat.z = in_quat.z / norm;
    out_quat.w = in_quat.w / norm;
    return out_quat;
}

mat4 quaternion_to_matrix(vec4 quat) {
    mat4 mat = mat4(0);

    float x = quat.x, y = quat.y, z = quat.z, w = quat.w;
    float x2 = x + x, y2 = y + y, z2 = z + z;
    float xx = x * x2, xy = x * y2, xz = x * z2;
    float yy = y * y2, yz = y * z2, zz = z * z2;
    float wx = w * x2, wy = w * y2, wz = w * z2;

    mat[0][0] = 1.0 - (yy + zz);
    mat[0][1] = xy - wz;
    mat[0][2] = xz + wy;
    mat[1][0] = xy + wz;
    mat[1][1] = 1.0 - (xx + zz);
    mat[1][2] = yz - wx;
    mat[2][0] = xz - wy;
    mat[2][1] = yz + wx;
    mat[2][2] = 1.0 - (xx + yy);
    mat[3][3] = 1.0;

    return mat;
}


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

    Transform global_transform_cur;
    for (int i = 0; i < global_keyframetrans_cur.length(); i++) {
        if (i == idx) {
            global_transform_cur = global_keyframetrans_cur[i];
            break;
        }
    }

    Transform global_transform_next;
    for (int i = 0; i < global_keyframetrans_next.length(); i++) {
        if (i == idx) {
            global_transform_next = global_keyframetrans_next[i];
            break;
        }
    }

    vec4 quaternion = mix(global_transform_cur.quaternion,
                          global_transform_next.quaternion,
                          keyframe);
    mat4 current_matrix = quaternion_to_matrix
        (global_transform_cur.quaternion);
    current_matrix[3] = vec4(mix(global_transform_cur.translation,
                                 global_transform_next.translation,
                                 keyframe), 1);


    vec4 vert_pos_inv = 1 *
        current_matrix *
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
