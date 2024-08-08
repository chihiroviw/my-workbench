#version 400 core

//今度からはfongシェーディングは proj_view + modelで切り分ける
//fsには，lightpos と cameraposをuniformで渡す
//proj + view+modelだとカメラは原点に来るから 楽だと思うが
//lightはview座標へ変換しなきゃだし，意外と手間は変わらん
//shadow map使うならなおさらproj_view + modelがいい

uniform mat4 view_model;
uniform mat4 proj;

uniform mat4 model;
uniform mat4 light_proj_view;

uniform mat3 normal_mat;

layout (location=0) in vec4 position;
layout (location=1) in vec4 color;
layout (location=2) in vec3 normal;


out vec4 view_model_position;
out vec3 view_model_normal;
out vec2 uv;

out vec4 light_proj_position;

void main() {
	//shadow
	light_proj_position = light_proj_view*model*position;

	//
	view_model_normal = normalize(normal_mat*normal);
	view_model_position = view_model*position;
	gl_Position = proj*view_model_position;


	/**** how to use gl_VertexID *****/
	if(gl_VertexID%3 == 0){
		uv = vec2(1, 1);
	}else if(gl_VertexID%3 == 1){
		uv = vec2(0, 1);
	}else{ //mod2
		uv = vec2(1, 0);
	}
}