#version 400 core

//���x�����fong�V�F�[�f�B���O�� proj_view + model�Ő؂蕪����
//fs�ɂ́Clightpos �� camerapos��uniform�œn��
//proj + view+model���ƃJ�����͌��_�ɗ��邩�� �y���Ǝv����
//light��view���W�֕ϊ����Ȃ��Ⴞ���C�ӊO�Ǝ�Ԃ͕ς���
//shadow map�g���Ȃ�Ȃ�����proj_view + model������

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