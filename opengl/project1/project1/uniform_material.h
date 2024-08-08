#pragma once
#include <memory>
#include <array>
#include <GLEW/glew.h>

// �ގ��f�[�^
struct Material{
	// �����̔��ˌW��
	alignas(16) std::array<GLfloat, 3> ambient;
	// �g�U���ˌW��
	alignas(16) std::array<GLfloat, 3> diffuse;
	// ���ʔ��ˌW��
	alignas(16) std::array<GLfloat, 3> specular;
	// �P���W��
	alignas(4) GLfloat shininess;
};

struct UniformBuffer{
	// ���j�t�H�[���o�b�t�@�I�u�W�F�N�g��
	GLuint ubo;

	// data: uniform �u���b�N�Ɋi�[����f�[�^
	UniformBuffer(const Material* data){
		// ���j�t�H�[���o�b�t�@�I�u�W�F�N�g���쐬����
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), data, GL_STATIC_DRAW);
	}

	~UniformBuffer(){
		glDeleteBuffers(1, &ubo);
	}
};


//class �� struct�ƈꏏ�Ńf�[�^�\����
//�R�s�[����ƒ��g���S���R�s�[�����
//shaered_ptr���R�s�[����邪�C�X�}�[�gptr�Ȃ̂ŃV�����[�R�s�[�ŁC
//�Q�ƃC���f�b�N�X��������邾��
//���̂�́Cshared_ptr buffer�𒼐ڃR�s�[����Ȃ�
//class uniform�̃R�s�[��shared_ptr buffer���R�s�[�����̂����҂���
class Uniform{
	const std::shared_ptr<UniformBuffer> buffer;

public:
	// data: uniform �u���b�N�Ɋi�[����f�[�^
	Uniform(Material* data = NULL)
		:buffer(new UniformBuffer(data)){}

	virtual ~Uniform(){}

	// ���j�t�H�[���o�b�t�@�I�u�W�F�N�g�Ƀf�[�^���i�[����
	// (�R���X�g���N�^��data==NULL�̎���z�肵��)
	// data: uniform �u���b�N�Ɋi�[����f�[�^
	void set(Material* data){
		glBindBuffer(GL_UNIFORM_BUFFER, buffer->ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), data);
	}

	// ���̃��j�t�H�[���o�b�t�@�I�u�W�F�N�g���g�p����
	// bp: �����|�C���g
	void select(GLuint bp){
		// �ގ��ɐݒ肷�郆�j�t�H�[���o�b�t�@�I�u�W�F�N�g���w�肷��
		glBindBufferBase(GL_UNIFORM_BUFFER, bp,	buffer->ubo);
	}
};
