#version 450
#extension GL_ARB_separate_shader_objects : enable

//input var
layout(location = 0) in vec3 frag_color;

//output var
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(frag_color, 1.0);
}