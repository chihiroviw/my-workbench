#pragma once
#include <stdio.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

class glfwWindow{
public:
    GLFWwindow* window_handler;

    //window info
    uint32_t size[2];
    float scale = 100.0f;

	//mouse 
    bool reset_mouse_offset = false;
	float mouse_location[2] = {0, 0};
	float mouse_offset[2] = {0, 0};

	
	//[W,A,S,D,space,X,R]
	bool key_response[7];

	//fps counter
	double last_time=0;
	int frame_count = 0;

    //fps limit
    double fps_limit  = 1.0 / 60.0;
    
    //
    bool enable_fps_count = false;

public:
    glfwWindow(uint32_t h=1200,uint32_t w=1920):size{w,h}{
        if(!glfwInit()) printf("ERROR: glfw init error.\n");
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        window_handler = glfwCreateWindow(w,h,"GLFW + VULKAN", NULL, NULL);
        
        if(!window_handler){        
            printf("ERROR: glfw window create error.\n");
            const char* err;
            glfwGetError(&err);
            printf("%s\n",err);
        }

		glfwSetWindowUserPointer(window_handler, this);

		glfwSetScrollCallback(window_handler, wheel);

		//glfwSetInputMode(window_handler, GLFW_CURSOR,GLFW_CURSOR_HIDDEN);
		glfwSetInputMode(window_handler, GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    }

    ~glfwWindow(){ glfwTerminate(); }
    
    void setInstanceExtention(std::vector<const char *>* inst_req_ext){
        uint32_t req_ext_count;
        const char ** req_ext = glfwGetRequiredInstanceExtensions(&req_ext_count);

        for(size_t i=0; i<req_ext_count; i++) 
            inst_req_ext->push_back(req_ext[i]);
    }
    
    VkSurfaceKHR getVkSurface(vk::UniqueInstance* instance){
        VkSurfaceKHR surface;
        VkResult result = glfwCreateWindowSurface(instance->get(), window_handler, nullptr, &surface);

        if (result != VK_SUCCESS) {
            const char* err;
            glfwGetError(&err);
            printf("getvksurface: %s\n",err);
        }

        return surface;
    }
    
    //return window is alive
	bool pollEvents(){
		glfwPollEvents();
		
		//mouse event
		double x, y;
		glfwGetCursorPos(window_handler, &x, &y);
			
        if(reset_mouse_offset){
            mouse_offset[0] = 0;
            mouse_offset[1] = 0;
            reset_mouse_offset = false;
        }
		mouse_offset[0] += (float)x - mouse_location[0];
		mouse_offset[1] += (float)y - mouse_location[1];

		mouse_location[0] = (float)x;
		mouse_location[1] = (float)y;
		
		//key response
		reset_key_response();
		if (glfwGetKey(window_handler, GLFW_KEY_W)) { key_response[0] = true; }
		if (glfwGetKey(window_handler, GLFW_KEY_A)) {key_response[1] = true;}
		if (glfwGetKey(window_handler, GLFW_KEY_S)) {key_response[2] = true;}
		if (glfwGetKey(window_handler, GLFW_KEY_D)) {key_response[3] = true;}
		if (glfwGetKey(window_handler, GLFW_KEY_SPACE)) {key_response[4] = true;}
		if (glfwGetKey(window_handler, GLFW_KEY_X)) {key_response[5] = true;}
		if (glfwGetKey(window_handler, GLFW_KEY_R)) {key_response[6] = true;}


		//window close
		if(	glfwWindowShouldClose(window_handler) || glfwGetKey(window_handler, GLFW_KEY_ESCAPE)){
			return false;
		}else {
			return true;
		}
	}
		

	bool* getKeyResponse() { return key_response; }

	uint32_t* getSize() {return size;}
    uint32_t w(){return size[0];}
    uint32_t h(){return size[1];}

	float getScale() {return scale;}

	float* getMouseLocation(){ return mouse_location; }	
	float* getMouseOffset(){ 
        reset_mouse_offset = true;
        return mouse_offset; 
    }	

	float fovy(){ return scale * 0.01f; }
	float aspect() { return (float)size[0] / (float)size[1]; }

	void printFps() {
		double current_time = glfwGetTime();
		frame_count++;
		if (current_time - last_time >= 1.0) {
			printf("%d frame/s\n", frame_count);
			frame_count= 0;
			last_time += 1.0;
		}
	}	
    
    void setFpsLimit(double limit){fps_limit = 1.0/limit;}
    void setEnableFpsCount(bool is){enable_fps_count = is;}
    
    void loop(std::function<void()> process){
        
        //fps limit
        double last_update_time = 0;    // number of seconds since the last loop
        double last_frame_time  = 0;    // number of seconds since the last frame
        
        while(pollEvents()){
            double now = glfwGetTime();
            double deltaTime = now - last_update_time;

            if ((now - last_frame_time) >= fps_limit){
                process();

                if(enable_fps_count)printFps();
                last_frame_time = now;
            }

            last_update_time = now;
        }
    }

private:	
	static void wheel(GLFWwindow* window, double x, double y) {
	    glfwWindow* ins = (glfwWindow*)glfwGetWindowUserPointer(window);

		if (ins != NULL) {
			ins->scale += (float)(y*10.0);
		}
	}

	void reset_key_response(void) {
		for (int i = 0; i < 7; i++) key_response[i] = false;
	}	
};