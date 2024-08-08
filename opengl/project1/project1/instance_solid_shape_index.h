#pragma once

#include "solid_shape_index.h"

class InsSolidShapeIndex : public SolidShapeIndex{
	GLuint pvm_vbo;

public:
	// size: 頂点の位置の次元
	// vertexcount: 頂点の数
	// vertex: 頂点属性を格納した配列
	// indexcount: 頂点のインデックスの要素数
	// index: 頂点のインデックスを格納した配列
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



		//最初の5秒だけデータ転送してみる
		static int first = 0;
		if (first < 1000){
			first++;
			glBindBuffer(GL_ARRAY_BUFFER, pvm_vbo);
			glBufferData(GL_ARRAY_BUFFER, instance_num*sizeof(Matrix), pvm, GL_STATIC_DRAW);
		}


		//どうも0がデフォルト臭い，もとに戻したくなったら0
		//0なら頂点ごとに参照要素をインクリメント
		//1以上なら,インスタンスごとに divisorの数ごとにインクリメント
		//1なら インスタンスごとに違うものを参照する
		//2なら 0 2 4の時に参照要素をインクリメント(隣り合った二つは同じものを参照)
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
