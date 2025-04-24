#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 frag_pos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    frag_pos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position  = projection * view * vec4(frag_pos, 1.0);
}

#shader fragment
#version 330 core
out vec4 frag_color;

in vec3 frag_pos;
in vec3 normal;

uniform vec3 object_color;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 view_pos;

void main() {
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    float specular_strength = 2.5;
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color;

    vec3 result = (ambient + diffuse + specular) * object_color;
    frag_color = vec4(result, 1.0);
}
