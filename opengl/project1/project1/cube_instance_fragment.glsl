#version 400 core

layout (location=0) out vec4 fragment;


//	I	= Ia + Is + Is
//	Ia	= Ka x La
//	Id	= max(N L, 0) x Kd x Ld
//	Is	= max(N H, 0) x Ks x Ls
//	H	= (L+V).normalize
//	L	= fragment->camera

uniform vec4 Lpos;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform vec3 La;
uniform vec3 Ld;
uniform vec3 Ls;

in vec4 vertex_color;
in vec3 frag_normal;
in vec2 uv;

void main() {

	fragment = vertex_color;
	//fragment = vertex_color*vec4(uv,1.0,1.0);
	//fragment = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);

	//fragment = vec4(0.3,0.3,0.3,1.0);
	//float r = gl_FragCoord.x/1000.0;
	//float g = gl_FragCoord.y/1000.0;
	//float b = gl_FragCoord.z/1000.0;
	//fragment = vec4(r,g,b,1.0);
}