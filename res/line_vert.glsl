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

layout(set = 0, binding = 0) uniform Ubo_Global_Camera {
    mat4 view_proj;
    vec3 pos;
}
ubo_camera;

layout(set = 1, binding = 0) readonly buffer Ssbo_Object_Model {
    mat4 model;
};
layout(set = 1, binding = 1) readonly buffer Ssbo_Object_InvBind {
    mat4 inverse_bind[];
};
layout(set = 1, binding = 2) readonly buffer Ssbo_Object_KeyframeTrans_Cur {
    Transform global_keyframetrans_cur[];
};
layout(set = 1, binding = 3) readonly buffer Ssbo_Object_KeyframeTrans_Next {
    Transform global_keyframetrans_next[];
};
layout(set = 1, binding = 4) readonly buffer Ssbo_Object_Keyframe {
    float keyframe;
};

layout(location = 0) out vec4 out_col;

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

    out_col = in_col;
    gl_Position = 1
        * current_matrix
        // * inverse_bind_cur
        * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
    gl_Position = 1
        * ubo_camera.view_proj
        * vec4(gl_Position.x, -gl_Position.y, gl_Position.z, gl_Position.w);
}
