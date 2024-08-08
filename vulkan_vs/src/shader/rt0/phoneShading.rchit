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
    
    //reflection
    pay_load.direction = reflect(pay_load.direction, world_normal);

    //limit recursive 
    pay_load.recursive -= 1;
    
    //
    pay_load.contribution_factor = 0.2;

    pay_load.hit_value = obj_param.custom_vec0.xyz;
}