#version 330 core

uniform vec3 view_position;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat4 light_view_proj_matrix;

out vec3 fragment_normal;
out vec3 fragment_position;
out vec4 fragment_position_light_space;

void main()
{
    fragment_normal = mat3(model_matrix) * normals;
    fragment_position = vec3(model_matrix * vec4(position, 1.0));
    fragment_position_light_space = light_view_proj_matrix * vec4(fragment_position, 1.0);
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);
}
