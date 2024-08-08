#pragma once

#include "solid_shape_index.h"

class InsSolidShapeIndex : public SolidShapeIndex{
	GLuint pvm_vbo;

public:
	// size: ���_�̈ʒu�̎���
	// vertexcount: ���_�̐�
	// vertex: ���_�������i�[�����z��
	// indexcount: ���_�̃C���f�b�N�X�̗v�f��
	// index: ���_�̃C���f�b�N�X���i�[�����z��
	InsSolidShapeIndex(GLint size, GLsizei vertexcount, Object::Vertex* vertex, GLsizei index_count, GLuint* index)
	:SolidShapeIndex(size, vertexcount,  vertex, index_count,  index){
		bind_vao();

		glGenBuffers(1, &pvm_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, pvm_vbo);

		/////////////////////////////////////
		glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), &(((Matrix*)0)->matrix[0]));
		glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), &(((Matrix*)0)->matrix[4]));
		glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), &(((Matrix*)0)->matrix[8]));
		glVertexAttribPointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), &(((Matrix*)0)->matrix[12]));
		glEnableVertexAttribArray(10);
		glEnableVertexAttribArray(11);
		glEnableVertexAttribArray(12);
		glEnableVertexAttribArray(13);


		glBindVertexArray(0);
	}

	void instance_elements_draw(int instance_num, Matrix* pvm){
		bind_vao();



		//�ŏ���5�b�����f�[�^�]�����Ă݂�
		static int first = 0;
		if (first < 1000){
			first++;
			glBindBuffer(GL_ARRAY_BUFFER, pvm_vbo);
			glBufferData(GL_ARRAY_BUFFER, instance_num*sizeof(Matrix), pvm, GL_STATIC_DRAW);
		}


		//�ǂ���0���f�t�H���g�L���C���Ƃɖ߂������Ȃ�����0
		//0�Ȃ璸�_���ƂɎQ�Ɨv�f���C���N�������g
		//1�ȏ�Ȃ�,�C���X�^���X���Ƃ� divisor�̐����ƂɃC���N�������g
		//1�Ȃ� �C���X�^���X���ƂɈႤ���̂��Q�Ƃ���
		//2�Ȃ� 0 2 4�̎��ɎQ�Ɨv�f���C���N�������g(�ׂ荇������͓������̂��Q��)
		glVertexAttribDivisor(0, 0);//vertex
		glVertexAttribDivisor(1, 0);//color

		glVertexAttribDivisor(10, 1);//mat4
		glVertexAttribDivisor(11, 1);//mat4
		glVertexAttribDivisor(12, 1);//mat4
		glVertexAttribDivisor(13, 1);//mat4
	

		glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, NULL, instance_num);

		glBindVertexArray(0);//unbind
	}
};
