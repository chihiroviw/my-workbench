#pragma once
#include <memory>
#include <GLEW/glew.h>

class TextureTest{
	GLuint texture_obj;
	int columns = 256, rows = 256;
	std::shared_ptr<char[]> tex_buffer;
	
public:
	TextureTest(void){
		glGenTextures(1, &texture_obj);
		glBindTexture(GL_TEXTURE_2D, texture_obj);

		gen_tex_buffer();
		glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA, columns, rows, 0, 
						GL_RGBA, GL_UNSIGNED_BYTE, tex_buffer.get());//source

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void bind(GLenum texture_unit){
		glActiveTexture(texture_unit);
		glBindTexture(GL_TEXTURE_2D, texture_obj);
	}

	void gen_tex_buffer(){
		tex_buffer = std::make_shared<char []>(columns*rows*4);

		bool is_red = false;
		for (int y = 0; y<columns; y++){
			if (y%32==0) is_red = !is_red;
			for (int x = 0; x<rows; x++){
				if (x%32==0) is_red = !is_red;

				int idx = (y*rows + x)*4;
				tex_buffer[idx+0] = is_red?255:0;//r
				tex_buffer[idx+1] = 0;//g
				tex_buffer[idx+2] = 0;//b
				tex_buffer[idx+3] = 1;//a
			}
		}
	}
};
