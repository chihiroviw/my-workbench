#version 400 core

//unused
uniform mat4 view_model;
uniform mat4 proj;
uniform mat3 normal_mat;

//used
uniform mat4 model;
uniform mat4 light_proj_view;

layout (location=0) in vec4 position;
layout (location=1) in vec4 color;
layout (location=2) in vec3 normal;


void main() {
	gl_Position = light_proj_view*model*position;
}