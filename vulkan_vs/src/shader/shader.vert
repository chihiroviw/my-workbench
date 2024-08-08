#version 450
#extension GL_ARB_separate_shader_objects : enable

//uniform
layout(set = 0, binding = 0) uniform sceneData{
    vec3 rectCenter;
} scene_data;

//input var
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;

//output var
layout(location = 0) out vec3 frag_color;

void main() {
    frag_color = in_color;
    gl_Position = vec4(in_pos+scene_data.rectCenter, 1.0);
}