#version 400 core

layout (location=0) out vec4 fragment;


/**shading**/
uniform vec4 Lpos;
uniform vec3 La;
uniform vec3 Ld;
uniform vec3 Ls;

layout (std140) uniform Material{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Ns;
};

/**texture**/
uniform sampler2D tex0;
uniform int use_tex;

void main() {
	//dummy このシェーダは使用されない(はず
}