#version 400 core

//uniform mat4 view_model;
uniform mat4 proj;

layout (location=0) in vec4 position;
layout (location=1) in vec4 color;
layout (location=2) in vec3 normal;

//model matrix
layout (location=10) in vec4 vm_m0;
layout (location=11) in vec4 vm_m1;
layout (location=12) in vec4 vm_m2;
layout (location=13) in vec4 vm_m3;


out vec4 vertex_color;
out vec3 frag_normal;
out vec2 uv;


void main() {
	mat4 vm;
	vm[0] = vm_m0; 
	vm[1] = vm_m1;
	vm[2] = vm_m2; 
	vm[3] = vm_m3;

	gl_Position = proj*vm*position;

	vertex_color = color;

	frag_normal = normal;

	/**** how to use gl_VertexID *****/
	if(gl_VertexID%3 == 0){
		uv = vec2(0, 0);
	}else if(gl_VertexID%3 == 1){
		uv = vec2(0, 1);
	}else{ //mod2
		uv = vec2(1, 0);
	}


	/***** how to use gl_InstanceID ****/
	//int dns = 100;
	//int dx = gl_InstanceID%dns;
	//int dy = (gl_InstanceID/dns)%dns;
	//int dz = (gl_InstanceID/(dns*dns))%dns;
	//vec4 dv = vec4(dx*7, dy*7, dz*7 ,0);
	//gl_Position = proj_view_model*(position+dv);
}