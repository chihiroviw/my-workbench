#pragma once
#include <cstdlib>
#include <vector>
#include <GLEW/glew.h>

class Object {
	GLuint vao;//vertex array object
	GLuint vbo;//vertex buffer object
	GLuint ibo;

public:
	//頂点属性
	struct Vertex {
		GLfloat position[3];
		GLfloat color[3];
		GLfloat normal[3];
	};

	// size			:頂点位置の次元
	// vertex_count	:頂点の数
	// vertex		:頂点属性を格納した配列
	// indexcount	:頂点のインデックスの要素数
	// index		:頂点のインデックスを格納した配列
	Object(GLint size, GLsizei vertex_count, const Vertex* vertex,
		GLsizei index_count = 0, GLuint *index = NULL) {

		//vertex array obj
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);//ここから下のbind_vboはこのvaoに対して行われる

		//vertex buffer obj
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);//ここから下はこのvboが処理される
		glBufferData(GL_ARRAY_BUFFER, vertex_count*sizeof(Vertex),vertex, GL_STATIC_DRAW);

		//結合されているvboをin変数から参照できるようにする
		glVertexAttribPointer(0, size, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((Vertex*)0)->position);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((Vertex*)0)->color);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((Vertex*)0)->normal);
		glEnableVertexAttribArray(2);


		// インデックスの頂点バッファオブジェクト
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), index, GL_STATIC_DRAW);

		glBindVertexArray(0);//unBind vao
	}

	void bind_vao() const{
		glBindVertexArray(vao);
	}

	virtual ~Object() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ibo);
	}

private:
	//disenable copy constractor
	Object(const Object& o);

	//disenable assaignment
	Object& operator=(const Object& o);
};


void load_obj_file(char* filename, std::vector<float[3]> vertex){

}
