#pragma once
#include <cstdlib>
#include <memory>

#include "object.h"

class Shape {
	//�}�`�f�[�^
public:
	std::shared_ptr<const Object> object;

protected:
	const GLsizei vertex_count;//�`��Ɏg�����_�̐�

public:
	// size			:���_�̈ʒu�̎���
	// vertexcount	:���_�̐�
	// vertex		:���_�������i�[�����z��
	// indexcount	:���_�̃C���f�b�N�X�̗v�f��
	// index		:���_�̃C���f�b�N�X���i�[�����z��
	Shape(GLint size, GLsizei vertex_count, const Object::Vertex* vertex,
		GLsizei index_count = 0, GLuint *index = NULL)
		: object(new Object(size, vertex_count, vertex, index_count, index))
		, vertex_count(vertex_count) {
	}

	void bind_vao() {
		object->bind_vao();
	}

	void draw() const {
		object->bind_vao();
		execute();
	}

	virtual void execute() const {
		glDrawArrays(GL_LINE_LOOP, 0, vertex_count);
		//glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	}
};
