#version 450 core

layout(location = 0) in vec3 in_pos;

layout(binding = 0) uniform Ubo_Camera {
    mat4 view_proj;
    vec3 pos;
}
ubo_camera;

layout(location = 0) out vec4 out_col;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    out_col = vec4(1,1,1,1);
    gl_Position = ubo_camera.view_proj
        // * ubo_model.model
        * vec4(in_pos.x, in_pos.y * -1, in_pos.z, 1.0);
}
