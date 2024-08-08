#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "memoryVma.hpp"

struct rtCamera{
    //camera move bias
    float sin80 = sin(80.0/360.0 * 2 * 3.14159265); //up_limit

	float origin_sensitivity[3] = { 30,30,30 };
	float lookat_sensitivity[2] = { 0.001/2,0.001/2};
    
    //init data
    glm::f32vec3 lookfrom_init;      
    glm::f32vec3 lookat_d_init;        
    glm::f32vec3 vup_init;           
    

    //const data
    uint32_t image_w,image_h;
    float focal_length;
    float viewport_height;
    float viewport_width; 
    
    //update par frame
    glm::f32vec3 lookfrom;      
    glm::f32vec3 lookat_d;        
    glm::f32vec3 vup;           

    //uniform buffer data
    //glm::f32vec3 lookfrom;      
    glm::f32vec3 pixel00_loc;
    glm::f32vec3 pixel_delta_u;    //horizon
    glm::f32vec3 pixel_delta_v;    //vertical
    
    //buffer for 
    uniqueBufferVma buffer;
    
    rtCamera(){}
    rtCamera(vulkanEngine*ve, uint32_t w, uint32_t h,
            glm::f32vec3 lookfrom_ = glm::f32vec3(0,0,1),
            glm::f32vec3 lookat_   = glm::f32vec3(0,0,0),
            float vfov              = 40,   //max:360
            glm::f32vec3 vup_      = glm::f32vec3(0,1,0)){

        image_w = w;
        image_h = h;
        auto aspect = (float)w/(float)h;

        focal_length = glm::length(lookat_ - lookfrom_);

        lookfrom = lookfrom_;
        lookfrom_init = lookfrom_;

        lookat_d = glm::normalize(lookat_-lookfrom_);
        lookat_d_init = glm::normalize(lookat_-lookfrom_);

        vup = vup_;
        vup_init = vup_;

        auto theta = (vfov)*3.141592/180;
        viewport_height = 2*tan(theta/2)*focal_length;
        viewport_width = viewport_height * aspect;
        
        buffer = std::move(uniqueBufferVma(ve,
                    sizeof(glm::f32vec4)*4,
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    VMA_MEMORY_USAGE_CPU_TO_GPU));
        
        updateViewport();
    }
    
    void updateBuffer(){
        
        void* dst = buffer.mapped_memory;
        
        const auto stride = sizeof(glm::f32vec4); 

        //padding vec3+4byte = vec4
        uint8_t data[stride*4];        
        std::memcpy(data            ,&lookfrom[0]       ,sizeof(glm::f32vec3));
        std::memcpy(data+stride*1   ,&pixel00_loc[0]    ,sizeof(glm::f32vec3));
        std::memcpy(data+stride*2   ,&pixel_delta_u[0]  ,sizeof(glm::f32vec3));
        std::memcpy(data+stride*3   ,&pixel_delta_v[0]  ,sizeof(glm::f32vec3));
        
        std::memcpy(dst ,data   ,stride*4);
    }

    void update(bool* key_response, float *mouse_offset){
        
		int dx = 0, dy = 0, dz = 0;
		if (key_response[0]) { dz += 1; } //w front
		if (key_response[1]) { dx += 1; } //a left
		if (key_response[2]) { dz -= 1; } //s back
		if (key_response[3]) { dx -= 1; } //d right
		if (key_response[4]) { dy += 1; } //space up 
		if (key_response[5]) { dy -= 1; } //x dawn

        
		updateOrigin(dx, dy, dz);
	
		updateLookat_d(mouse_offset);

		if (key_response[6]) { reset(); }
        
        updateViewport();
    }

private:
    //x y z must == (0 or -1 or 1)
	void updateOrigin(int dx, int dy, int dz) {

		//left right
		if (dx != 0) {
			float fdx = dx*origin_sensitivity[0];
			auto right = cross(vup,lookat_d);
			glm::normalize(right);
			lookfrom += right*fdx;
		}

		//up down
		if (dy != 0) {
			float fdy = dy*origin_sensitivity[1];
			lookfrom += vup*fdy;
		}

		//flont back
		if (dz != 0) {
			float fdz = dz*origin_sensitivity[2];
			auto d = glm::normalize(lookat_d);
			lookfrom += d*fdz;
		}	
	}

	//mdx, mdy : pixel
	void updateLookat_d(float* mo) {

		float m_dx = mo[0], m_dy = mo[1];

		if (m_dx != 0) {
			float fdx = m_dx*lookat_sensitivity[0];

			auto left = glm::cross(lookat_d,vup);
			glm::normalize(left);
			lookat_d    += left*fdx;
			glm::normalize(lookat_d);
		}

        auto old_lookat_d = lookat_d;

		if (m_dy != 0) {
			float fdy   = m_dy*lookat_sensitivity[1];
			auto left   = glm::cross(lookat_d,vup);
			auto tup    = glm::cross(lookat_d,left);
			lookat_d    += tup * fdy;
			glm::normalize(lookat_d);
		}
		
		//limit camera y angle
		if (lookat_d[1] > sin80 ) {
            lookat_d[1] = sin80;
		}

		if (-sin80 > lookat_d[1]) {
            lookat_d[1] = -sin80;
		}
	}	
    
    void updateViewport(){
        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        auto w = glm::normalize(-lookat_d);
        auto u = glm::normalize(glm::cross(vup,w));
        auto v = glm::cross(w,u);
 
        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * -v;

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / float(image_w);
        pixel_delta_v = viewport_v / float(image_h);

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = lookfrom
                             - focal_length*w
                             - viewport_u/2.0f 
                             - viewport_v/2.0f;

        pixel00_loc = viewport_upper_left 
                    + 0.5f * (pixel_delta_u + pixel_delta_v);          
    }
    
    void reset(){
        lookfrom = lookfrom_init;
        lookat_d = lookat_d_init;
        vup      = vup_init;
    }
};