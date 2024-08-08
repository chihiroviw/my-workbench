#pragma once
#include "shape_index.h"

class SolidShapeIndex: public ShapeIndex{
public:

	// size: 頂点の位置の次元
	// vertexcount: 頂点の数
	// vertex: 頂点属性を格納した配列
	// indexcount: 頂点のインデックスの要素数
	// index: 頂点のインデックスを格納した配列
	SolidShapeIndex(GLint size, GLsizei vertexcount, Object::Vertex* vertex,
		GLsizei index_count, GLuint* index)
		: ShapeIndex(size, vertexcount, vertex, index_count, index){}

	// 描画の実行
	virtual void execute() const{
		// 三角形で描画する
		glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
	}
};
