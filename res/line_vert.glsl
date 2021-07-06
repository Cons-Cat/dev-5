#version 450 core

layout(location = 0) in vec3 in_pos;

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

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    out_col = vec4(1,1,1,1);
    gl_Position = ubo_camera.projection  * ubo_camera.view
        * ubo_model.model * vec4(in_pos.x, in_pos.y * -1, in_pos.z, 1.0);
}
