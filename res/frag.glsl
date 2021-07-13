#version 450 core

layout(binding = 2) uniform Ubo_Camera {
    mat4 projection;
    mat4 view;
}
ubo_camera;

layout(binding = 3) uniform sampler2D diffuse_color_map;
layout(binding = 4) uniform sampler2D emissive_color_map;
layout(binding = 5) uniform sampler2D normal_color_map;
layout(binding = 6) uniform sampler2D specular_color_map;

layout(location = 0) in vec4 in_col;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_pos;

layout(location = 0) out vec4 out_color;

// Light source
vec3 light_pos = vec3(10, -10, 20) * 3;
vec3 ambience = vec3(0.25, 0.25, 0.25);

// Material properties
float shininess = 30.26;

void main() {
    vec3 diffuse_color = texture(diffuse_color_map, in_uv).rgb;
    vec3 specular_color = texture(specular_color_map, in_uv).rgb;
    vec3 normal_color = normalize(texture(normal_color_map, in_uv).rgb);

    vec3 light_dir = light_pos - in_pos;
    light_dir = normalize(light_dir);
    float lambertian_intensity = max(dot(light_dir, normal_color), 0.0);

    // Draw vector from <0,0,0> (the view pos) to the vertex coordinates.
    vec3 view_dir = normalize(-in_pos);
    vec3 half_dir = normalize(light_dir + view_dir);
    float specular_intensity = pow(max(dot(normal_color, half_dir), 0),
                                    shininess);

    vec3 ambient = ambience * diffuse_color;
    vec3 diffuse = lambertian_intensity * diffuse_color;
    vec3 specular = specular_intensity * specular_color;
    diffuse -= specular;
    diffuse = max(diffuse, 0);
    out_color = vec4(ambient + specular + diffuse, 1);
}
