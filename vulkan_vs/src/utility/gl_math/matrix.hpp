#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

void test(){
    glm::f32vec3 v0 = glm::normalize(glm::vec3(2));
    glm::f32vec3 v1 = glm::vec3(3);

    glm::f32vec3 v = glm::cross(v0,v1);

    printf("\n%s\n",glm::to_string(v0).data());
    
    glm::mat4x4 m44_0,m44_1;
    auto r = m44_0*m44_1;    
}

