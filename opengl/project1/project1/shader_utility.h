#pragma once
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>

void read_shader_source(std::vector<std::string>* vec, char* file_name);

GLboolean compile_shader_successful(GLuint shader, GLenum shader_type);
void print_shader_info_log(GLuint shader, GLenum shader_type);

GLboolean link_program_successful(GLuint program);
void print_program_info_log(GLuint program);

void compile_and_attach_shader(
	std::vector<std::string>* src, GLuint program, GLenum shader_type,const char* shader_name);

GLuint load_create_program(const char* v_file, const char* f_file);

void read_shader_source(
	std::vector<std::string>* vec,
	char* file_name) {
	
	std::ifstream file = std::ifstream(file_name);
	if (!file) {
		printf("cant open %s\n", file_name);
		exit(1);
	}

	while (true) {
		//こっちでもいい
		//vec->push_back(std::string());
		//if (!std::getline(file, vec->back())){ break; }

		auto line = std::string();
		if (!std::getline(file, line)) {break;}
		line += '\n';
		vec->push_back(std::move(line));
	}
}

GLboolean compile_shader_successful(GLuint shader, GLenum shader_type) {
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	return (GLboolean)status;
}

void print_shader_info_log(GLuint shader, GLenum shader_type) {
	GLsizei  buf_size;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &buf_size);

	if (buf_size> 1) {
		std::vector<GLchar> info_log(buf_size);
		GLsizei length;
		glGetShaderInfoLog(shader, buf_size, &length, &info_log[0]);
		printf("%s\n", &info_log[0]);
	}
}

GLboolean link_program_successful(GLuint program) {
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	return (GLboolean)status;
}

void print_program_info_log(GLuint program) {
	GLsizei buf_size;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buf_size);

	if (buf_size > 1) {
		std::vector<GLchar> info_log(buf_size);
		GLsizei length;
		glGetProgramInfoLog(program, buf_size, &length, &info_log[0]);
		printf("%s\n", &info_log[0]);
	}
}

void compile_and_attach_shader(
	std::vector<std::string>* src, 
	GLuint program, GLenum shader_type, 
	const char* shader_name) {

	auto p_src = std::make_unique<char*[]>(src->size());
	for (int i = 0; i < src->size(); i++) {
		p_src[i] = (char*)(*src)[i].c_str();
	}

	const GLuint shader_obj = glCreateShader(shader_type);	
	glShaderSource(shader_obj, src->size(), p_src.get(), NULL);
	glCompileShader(shader_obj);

	if (compile_shader_successful(shader_obj, shader_type)) {
		glAttachShader(program, shader_obj);

	}else {
		printf("/*** shader compile error in %s ***/\n",shader_name);
		print_shader_info_log(shader_obj, shader_type);
	}

	glDeleteShader(shader_obj);
}

GLuint load_create_program(const char* v_file, const char* f_file) {
	GLuint program = glCreateProgram();

	// compile + link vertex shader
	std::vector<std::string> v_src;
	read_shader_source(&v_src, (char*)v_file);
	compile_and_attach_shader(&v_src, program, GL_VERTEX_SHADER,"vertex_shader");

	// compile + link flagment shader
	std::vector<std::string> f_src;
	read_shader_source(&f_src, (char*)f_file);
	compile_and_attach_shader(&f_src, program, GL_FRAGMENT_SHADER,"fragment_shader");
	
	// プログラム中の入出力変数を名前で指定して，登録
	//glBindAttribLocation(program, 0, "position");
	//glBindAttribLocation(program, 1, "color");
	//glBindFragDataLocation(program, 0, "fragment");
	
	//link program object
	glLinkProgram(program);

	// 作成したプログラムオブジェクトを返す
	if (link_program_successful(program)) {
		return program;
	}else {	
		printf("link program object error\n");
		print_program_info_log(program);
		glDeleteProgram(program);
		return 0;
	}
}














/*
void read_shader_source(
	std::vector<std::string>* vec,
	char* file_name) {
	
	std::ifstream file = std::ifstream(file_name);
	if (!file) {
		printf("cant open %s\n", file_name);
		exit(1);
	}

	while (true) {
		//こっちでもいい
		//vec->push_back(std::string());
		//if (!std::getline(file, vec->back())){ break; }

		auto line = std::string();
		if (!std::getline(file, line)) {break;}
		line += '\n';
		vec->push_back(std::move(line));
	}
}
	//translate vector<strint> -> char**
	auto p_src = std::make_unique<char*[]>(src->size());
	for (int i = 0; i < src->size(); i++) {
		p_src[i] = (char*)(*src)[i].c_str();
	}
*/


/*
void read_file_to_lines(
	std::vector<std::unique_ptr<std::string>>* vec,
	char* file_name) {
	
	std::ifstream file = std::ifstream(file_name);
	if (!file) {
		printf("cant open %s\n", file_name);
		exit(1);
	}

	while (true) {
		auto line = std::make_unique<std::string>();
		if (!std::getline(file, *line)) {break;}

		vec->push_back(std::move(line));
	}
}*/
