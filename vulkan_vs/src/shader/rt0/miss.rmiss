#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../rtUtility/rtCommon.glsl"

layout(location = 0) rayPayloadInEXT hitPayload pay_load;

void main(){
    pay_load.recursive = 0;
    pay_load.hit_value = vec3(0.0);
    pay_load.contribution_factor = 0.0;
}