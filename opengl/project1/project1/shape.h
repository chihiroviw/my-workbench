#pragma once
#include <cstdlib>
#include <memory>

#include "object.h"

class Shape {
	//図形データ
public:
	std::shared_ptr<const Object> object;

protected:
	const GLsizei vertex_count;//描画に使う頂点の数

public:
	// size			:頂点の位置の次元
	// vertexcount	:頂点の数
	// vertex		:頂点属性を格納した配列
	// indexcount	:頂点のインデックスの要素数
	// index		:頂点のインデックスを格納した配列
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
