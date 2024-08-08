#pragma once
#include <iostream>
#include <cstdlib>
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>

class Window {
	GLFWwindow* window = NULL;
	GLfloat size[2];
	GLfloat scale = 100.0f;

	//mouse 
	GLfloat mouse_location[2] = {0, 0};
	GLfloat mouse_offset[2] = {0, 0};
	
	//[W,A,S,D,space,X,R]
	bool key_response[7];

	//var for fps counter
	double last_time=0;
	int frame_count = 0;

public:
	Window(	int width = 1920, int height = 1080, 
			const char* title = "glfw window") {

		init_glfw();
		set_callback();
		opengl_version(4, 2);

		window = glfwCreateWindow(width, height, title, NULL, NULL);
		if (window == NULL) {
			printf("Can't create GLFW window.\n");
			exit(1);
		}

		glfwMakeContextCurrent(window);
		init_glew();
		glfwSwapInterval(1);

		glfwSetWindowUserPointer(window, this);

		glfwSetWindowSizeCallback(window, resize);

		glfwSetScrollCallback(window, wheel);

		//glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_HIDDEN);
		glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_DISABLED);

		resize(window, width, height);

		glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

		// 背面カリングを有効にする
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		// デプスバッファを有効にする
		glClearDepth(1.0);
		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);
	}

	virtual ~Window() {
		glfwDestroyWindow(window);
	}

	//return window is alive
	bool poll_events(){
		glfwPollEvents();
		
		//mouse event
		double x, y;
		glfwGetCursorPos(window, &x, &y);
			
		mouse_offset[0] = x - mouse_location[0];
		mouse_offset[1] = y - mouse_location[1];

		mouse_location[0] = x;
		mouse_location[1] = y;
		
		//key response
		reset_key_response();
		if (glfwGetKey(window, GLFW_KEY_W)) { key_response[0] = true; }
		if (glfwGetKey(window, GLFW_KEY_A)) {key_response[1] = true;}
		if (glfwGetKey(window, GLFW_KEY_S)) {key_response[2] = true;}
		if (glfwGetKey(window, GLFW_KEY_D)) {key_response[3] = true;}
		if (glfwGetKey(window, GLFW_KEY_SPACE)) {key_response[4] = true;}
		if (glfwGetKey(window, GLFW_KEY_X)) {key_response[5] = true;}
		if (glfwGetKey(window, GLFW_KEY_R)) {key_response[6] = true;}


		//window close
		if(	glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)){
			return false;
		}else {
			return true;
		}
	}
	

	void swap_buffers() const {
		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	bool* get_key_response() { return key_response; }

	GLfloat* get_size() {return size;}

	GLfloat get_scale() {return scale;}

	GLfloat* get_mouse_location(){ return mouse_location; }	
	GLfloat* get_mouse_offset(){ return mouse_offset; }	

	GLfloat fovy(){ return scale * 0.01; }
	GLfloat aspect() { return size[0] / size[1]; }

	void print_fps() {
		double current_time = glfwGetTime();
		frame_count++;
		if (current_time - last_time >= 1.0) {
			printf("%d frame/s\n", frame_count);
			//printf("%f %f\n", mouse_location[0], mouse_location[1]);
			frame_count= 0;
			last_time += 1.0;
		}
	}

	void reset_viewport(){
		int fb_width, fb_height;
		glfwGetFramebufferSize(window, &fb_width, &fb_height);
		glViewport(0, 0, fb_width, fb_height);
	}

private:
	static void resize(GLFWwindow* window, int width, int height) {
		int fb_width, fb_height;
		glfwGetFramebufferSize(window, &fb_width, &fb_height);

		glViewport(0, 0, fb_width, fb_height);

		Window* ins = (Window*)glfwGetWindowUserPointer(window);

		if (ins != NULL) {
			ins->size[0] = (GLfloat)width;
			ins->size[1] = (GLfloat)height;
		}
	}

	static void wheel(GLFWwindow* window, double x, double y) {
		Window* ins = (Window*)glfwGetWindowUserPointer(window);

		if (ins != NULL) {
			ins->scale += (float)(y*10.0);
		}
	}

	void reset_key_response(void) {
		for (int i = 0; i < 7; i++) key_response[i] = false;
	}

	void init_glew(void) {
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK) {
			printf("initialize GLEW failed\n");
			exit(1);
		}
	}

	void opengl_version(int majar, int minor) {
		// デフォルトの設定に戻すには glfwDefaultWindowHints() を呼び出します。
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majar);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	void set_callback(void) {
		atexit(glfwTerminate);
	}

	void init_glfw(void) {
		if (glfwInit() == GL_FALSE) {
			printf("initialize glfw failed\n");
			exit(1);
		}
	}
};


