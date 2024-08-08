#pragma once
#include "shape_index.h"

class SolidShapeIndex: public ShapeIndex{
public:

	// size: ���_�̈ʒu�̎���
	// vertexcount: ���_�̐�
	// vertex: ���_�������i�[�����z��
	// indexcount: ���_�̃C���f�b�N�X�̗v�f��
	// index: ���_�̃C���f�b�N�X���i�[�����z��
	SolidShapeIndex(GLint size, GLsizei vertexcount, Object::Vertex* vertex,
		GLsizei index_count, GLuint* index)
		: ShapeIndex(size, vertexcount, vertex, index_count, index){}

	// �`��̎��s
	virtual void execute() const{
		// �O�p�`�ŕ`�悷��
		glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
	}
};
