#version 450 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_col;

layout(set = 0, binding = 0) uniform Ubo_Camera {
    mat4 view_proj;
    vec3 pos;
}
ubo_camera;

layout(set = 1, binding = 0) readonly buffer Ubo_Object_Model {
    mat4 model;
};
layout(set = 1, binding = 1) readonly buffer Ubo_Object_InvBind {
    mat4 inverse_bind[27];
};
layout(set = 1, binding = 2) readonly buffer Ubo_Object_GlobTrans {
    mat4 global_transform[27];
};
layout(set = 1, binding = 3) readonly buffer Ubo_Object_Weights {
    float weight[27];
};

layout(location = 0) out vec4 out_col;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    int idx = gl_VertexIndex;
    out_col = in_col;
    gl_Position = 1
        * ubo_camera.view_proj
        * global_transform[idx]
        * inverse_bind[idx]
        * vec4(in_pos.x, in_pos.y * -1, in_pos.z, 1.0);
}
