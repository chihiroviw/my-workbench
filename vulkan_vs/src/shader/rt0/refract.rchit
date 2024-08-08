#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../rtUtility/rtCommon.glsl"
#include "../rtUtility/fetchVertex.glsl"

layout(location = 0) rayPayloadInEXT hitPayload pay_load;

hitAttributeEXT vec3 attribs;

void main(){
    vec3 barys = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    objectParameter obj_param = obj_params[gl_InstanceCustomIndexEXT];

    vertexPNX vert_pnx = fetchVertexInterleavedPNX( barys, 
                                                    obj_param.vert_addr,
                                                    obj_param.idx_addr);
                                                    
    //world position
    pay_load.origin = gl_ObjectToWorldEXT * vec4(vert_pnx.position,1);

    //world normal
    vec3 world_normal = mat3(gl_ObjectToWorldEXT) *vert_pnx.normal;
    
    const float refract_value = 2.4;
    float eta;
    float nr = dot(world_normal, pay_load.direction);
    vec3 orienting_normal;

    if( nr < 0) {
      // 表面. 空気中 -> 屈折媒質.
      eta = 1.0 / refract_value;
      orienting_normal = world_normal;
    } else {
      // 裏面. 屈折媒質 -> 空気中.  
      eta = refract_value / 1.0;
      orienting_normal = -world_normal;
    }

    pay_load.direction = refract(pay_load.direction, orienting_normal , eta);

    if(length(pay_load.direction)<0.01) {
      // 全反射している.
      pay_load.direction = reflect(pay_load.direction, orienting_normal);
    }

    //limit recursive 
    pay_load.recursive -= 1;
    
    //
    pay_load.contribution_factor = 1;

    pay_load.hit_value = vec3(0.0);
}