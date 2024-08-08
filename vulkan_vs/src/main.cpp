#include <stdio.h>

#include "vk_include/link.h"
//#include "vk_include/rasterVulkanEngine.hpp"
#include "vk_include/rtVulkanEngine.hpp"



//struct class function     -> caramelCase
//var                       -> snake_case
int main(void) {
    glfwWindow window;
    window.setFpsLimit(30.0);

    //rasterVulkanEngine vk_engine(&window);  
    //vk_engine.run();     
    
    rtVulkanEngine vk_engine(&window);
    vk_engine.run();
    
    return 0;
}

