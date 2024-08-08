#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_buffer_reference: enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

//---------------------------
//  structures
//---------------------------

struct hitPayload{
    vec3 hit_value;
    float contribution_factor;
    int  recursive;
    vec3 origin;
    vec3 direction;//normalized 
};

struct objectParameter{
    uint64_t vert_addr;
    uint64_t idx_addr;
    uint64_t custom_idx0;
    uint64_t custom_idx1;
    vec4    custom_vec0;
    vec4    custom_vec1;
    vec4    custom_vec2;
    vec4    custom_vec3;
};

//---------------------------
//  descriptor bindings
//---------------------------

//uniform buffer
layout(set = 0, binding = 0) uniform cameraInfo{
    vec4 lookfrom;      
    vec4 pixel00_loc;
    vec4 pixel_delta_u; //horizon
    vec4 pixel_delta_v; //vertical
}camera_info;

layout(set = 0, binding = 1) readonly buffer objectParams{
    objectParameter obj_params[];
};

//TLAS
layout(set = 1, binding = 0) uniform accelerationStructureEXT topLevelAS;

//output image
layout(set = 2, binding = 0, rgba8) uniform image2D image;


