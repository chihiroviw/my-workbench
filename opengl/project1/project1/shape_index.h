#pragma once
#include "gl_matrix.h"
#include "shape.h"

// �C���f�b�N�X���g�����}�`�̕`��
class ShapeIndex: public Shape {
protected:
	// �`��Ɏg�����_�̐�
	const GLsizei index_count;

public:
	// �R���X�g���N�^
	// size			:���_�̈ʒu�̎���
	// vertexcount	:���_�̐�
	// vertex		:���_�������i�[�����z��
	// indexcount	:���_�̃C���f�b�N�X�̗v�f��
	// index		:���_�̃C���f�b�N�X���i�[�����z��
	ShapeIndex(GLint size, GLsizei vertex_count, const Object::Vertex* vertex,
		GLsizei index_count, GLuint* index)
		: Shape(size, vertex_count, vertex, index_count, index)
		, index_count(index_count){
	}


	virtual void execute() const{
		glDrawElements(GL_LINES, index_count, GL_UNSIGNED_INT, 0);
	}
};
