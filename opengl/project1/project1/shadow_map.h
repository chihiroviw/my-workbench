#pragma once
#include <memory>
#include <GLEW/glew.h>

class ShadowMapFbo{
	GLuint fbo;
	GLuint shadow_map_tex;
	int size;

public:
	ShadowMapFbo(int s=2000){
		
		size = s;
		glGenTextures(1, &shadow_map_tex);
		glBindTexture(GL_TEXTURE_2D, shadow_map_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, s, s, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		//fboにtexをbindする専用関数
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map_tex, 0);
		//描画するカラーバッファーを指定します(GL_NONE カラーバッファなし,フラグメントシェーダは使われない
		glDrawBuffer(GL_NONE);
		//後続のglReadPixels関数(など)で使用するバッファを指定
		glReadBuffer(GL_NONE);

		GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (Status != GL_FRAMEBUFFER_COMPLETE){
			printf("FB error, status: 0x%x\n", Status);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void bind_for_writing(){
		glViewport(0, 0, size, size);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	}
	void bind_for_reading(GLenum TextureUnit){
		glActiveTexture(TextureUnit);
		glBindTexture(GL_TEXTURE_2D, shadow_map_tex);
	}
};
