#version 450 core

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
layout(set = 1, binding = 2) readonly buffer Ssbo_Object_GlobTrans {
    mat4 global_transform[];
};
layout(set = 1, binding = 3) readonly buffer Ssbo_Object_Weights {
    float weight[];
};

layout(location = 0) out vec4 out_col;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    int idx = gl_VertexIndex / 2;

    mat4 inverse_bind_cur;
    for (int i = 0; i < inverse_bind.length(); i++) {
        if (i == idx) {
            inverse_bind_cur = inverse_bind[i];
            break;
        }
    }

    mat4 global_transform_cur;
    for (int i = 0; i < global_transform.length(); i++) {
        if (i == idx) {
            global_transform_cur = global_transform[i];
            break;
        }
    }

    out_col = in_col;
    gl_Position = 1
        * ubo_camera.view_proj
        * global_transform_cur
        * inverse_bind_cur
        * vec4(in_pos.x, in_pos.y * -1, in_pos.z, 1.0);
}
