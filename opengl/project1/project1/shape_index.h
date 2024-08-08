#pragma once
#include "gl_matrix.h"
#include "shape.h"

// インデックスを使った図形の描画
class ShapeIndex: public Shape {
protected:
	// 描画に使う頂点の数
	const GLsizei index_count;

public:
	// コンストラクタ
	// size			:頂点の位置の次元
	// vertexcount	:頂点の数
	// vertex		:頂点属性を格納した配列
	// indexcount	:頂点のインデックスの要素数
	// index		:頂点のインデックスを格納した配列
	ShapeIndex(GLint size, GLsizei vertex_count, const Object::Vertex* vertex,
		GLsizei index_count, GLuint* index)
		: Shape(size, vertex_count, vertex, index_count, index)
		, index_count(index_count){
	}


	virtual void execute() const{
		glDrawElements(GL_LINES, index_count, GL_UNSIGNED_INT, 0);
	}
};
