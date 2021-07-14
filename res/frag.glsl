#version 450 core

// layout(binding = 2) uniform Ubo_Camera {
//     mat4 projection;
//     mat4 view;
// }
// ubo_camera;

layout(binding = 3) uniform sampler2D diffuse_color_map;
layout(binding = 4) uniform sampler2D emissive_color_map;
layout(binding = 5) uniform sampler2D normal_color_map;
layout(binding = 6) uniform sampler2D specular_color_map;

layout(location = 0) in vec4 in_col;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_pos_vert;
layout(location = 3) in vec4 in_pos_view;
layout(location = 4) in vec3 in_norm;

layout(location = 0) out vec4 out_color;

// Light source
vec3 light_pos = vec3(0.5, -7, 5);
vec3 ambience = vec3(0.25, 0.25, 0.25);

// Material properties
float shininess;

void main() {
    vec3 diffuse_color = texture(diffuse_color_map, in_uv).rgb;
    vec3 specular_color = texture(specular_color_map, in_uv).rgb;
    vec3 normal_color = texture(normal_color_map, in_uv).xyz;
    vec4 emissive_color = texture(emissive_color_map, in_uv).rgba;

    vec3 normal = (in_norm.xyz);
    float light_intensity = 2;
    // Higher should be smaller highlight
    shininess = 1.32*1;

    vec3 light_dir = normalize(light_pos.xyz - in_pos_vert.xyz);
    float lambertian_intensity = max(dot(light_dir, normal), 0.0);
    float specular_strength = 10;

    vec3 ambient = ambience * diffuse_color;
    vec3 diffuse = lambertian_intensity * diffuse_color * light_intensity;
    vec4 emissive = emissive_color;

    if (lambertian_intensity > 0.0){
        vec3 view_dir = normalize(in_pos_view.xyz - in_pos_vert.xyz);
        vec3 half_dir = normalize(light_dir + view_dir);
        float spec_angle = max(dot(half_dir, normal), 0);
        float specular_intensity = pow(spec_angle, shininess) * specular_strength;

        vec3 specular = specular_intensity * specular_color * light_intensity;
        diffuse -= specular;
        diffuse = max(diffuse, 0);
        out_color = vec4(ambient + diffuse + specular, 1) + emissive;
        return;
    }
    out_color = vec4(ambient + diffuse, 1) + emissive;
}
