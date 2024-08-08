#pragma once
#include <memory>
#include <array>
#include <GLEW/glew.h>

// 材質データ
struct Material{
	// 環境光の反射係数
	alignas(16) std::array<GLfloat, 3> ambient;
	// 拡散反射係数
	alignas(16) std::array<GLfloat, 3> diffuse;
	// 鏡面反射係数
	alignas(16) std::array<GLfloat, 3> specular;
	// 輝き係数
	alignas(4) GLfloat shininess;
};

struct UniformBuffer{
	// ユニフォームバッファオブジェクト名
	GLuint ubo;

	// data: uniform ブロックに格納するデータ
	UniformBuffer(const Material* data){
		// ユニフォームバッファオブジェクトを作成する
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), data, GL_STATIC_DRAW);
	}

	~UniformBuffer(){
		glDeleteBuffers(1, &ubo);
	}
};


//class も structと一緒でデータ構造体
//コピーすると中身が全部コピーされる
//shaered_ptrもコピーされるが，スマートptrなのでシャローコピーで，
//参照インデックスが一つ増えるだけ
//下のやつは，shared_ptr bufferを直接コピーじゃなく
//class uniformのコピーでshared_ptr bufferがコピーされるのを期待する
class Uniform{
	const std::shared_ptr<UniformBuffer> buffer;

public:
	// data: uniform ブロックに格納するデータ
	Uniform(Material* data = NULL)
		:buffer(new UniformBuffer(data)){}

	virtual ~Uniform(){}

	// ユニフォームバッファオブジェクトにデータを格納する
	// (コンストラクタでdata==NULLの時を想定して)
	// data: uniform ブロックに格納するデータ
	void set(Material* data){
		glBindBuffer(GL_UNIFORM_BUFFER, buffer->ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), data);
	}

	// このユニフォームバッファオブジェクトを使用する
	// bp: 結合ポイント
	void select(GLuint bp){
		// 材質に設定するユニフォームバッファオブジェクトを指定する
		glBindBufferBase(GL_UNIFORM_BUFFER, bp,	buffer->ubo);
	}
};
