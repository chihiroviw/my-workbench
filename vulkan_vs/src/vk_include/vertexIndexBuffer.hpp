#pragma once
#include "vulkanEngine.hpp"
#include "memoryVma.hpp"
#include <glm/glm.hpp>


//----------------------
//  struct basic vertex + index
//----------------------
struct elementBuffer{

    vulkanEngine* ve;
    uniqueBufferVma buffer;
    uint32_t stride, elem_count;

    elementBuffer(){}
    elementBuffer(vulkanEngine*ve, 
                void* data,
                uint32_t stride,
                uint32_t elem_count,
                vk::BufferUsageFlags usage)
                :ve(ve),elem_count(elem_count),stride(stride){

        buffer  = uniqueBufferVma(ve,
                        elem_count*stride,
                        data,
                        usage,
                        VMA_MEMORY_USAGE_GPU_ONLY);
    }     
    
    uint32_t bytes(){return stride*elem_count;}
};

struct vertexBuffer:public elementBuffer{
    vertexBuffer(){}
    vertexBuffer(vulkanEngine* ve, 
                void* verts_data,
                uint32_t stride,
                uint32_t elem_count,
                vk::BufferUsageFlags optional_usage = {})
                :elementBuffer(ve,
                        verts_data,
                        stride,
                        elem_count,
                        vk::BufferUsageFlagBits::eVertexBuffer
                        |optional_usage){} 
    
    uint32_t maxVertex(){return elem_count;}
    vk::Format format(){return vk::Format::eR32G32B32Sfloat;}
};

struct indexBuffer:public elementBuffer{
    indexBuffer(){}
    indexBuffer(vulkanEngine* ve, 
                void* index_data,
                uint32_t elem_count,
                vk::BufferUsageFlags optional_usage = {})
                :elementBuffer(ve,
                            index_data,
                            sizeof(uint32_t),
                            elem_count,
                            vk::BufferUsageFlagBits::eIndexBuffer
                            |optional_usage){} 

    vk::IndexType format(){return vk::IndexType::eUint32;}
};



struct uniquePolygonMesh{
    vertexBuffer vert_buf;
    indexBuffer  idx_buf;

    struct objectParameter{
        uint64_t vert_addr;
        uint64_t idx_addr;
        uint64_t custom_idx0;
        uint64_t custom_idx1;
        glm::f32vec4 custom_vec0;
        glm::f32vec4 custom_vec1;
        glm::f32vec4 custom_vec2;
        glm::f32vec4 custom_vec3;
    }object_parameter;
    
public:
    uniquePolygonMesh(){}
    uniquePolygonMesh(vulkanEngine* ve,
                void* verts_data, uint32_t vert_stride, uint32_t verts_count,
                void* index_data, uint32_t index_count,
                vk::BufferUsageFlags optional_usage = {})

                :vert_buf(ve,verts_data,vert_stride,verts_count,optional_usage),
                idx_buf(ve,index_data,index_count,optional_usage),
                object_parameter({vert_buf.buffer.address,idx_buf.buffer.address}){}

    void setParameterIndex(uint64_t u0, uint64_t u1=0){
        object_parameter.custom_idx0 = u0;
        object_parameter.custom_idx1 = u0;
    }
    
    void setParameterVector(glm::vec4 v0, 
                            glm::vec4 v1 = {}, 
                            glm::vec4 v2 = {}, 
                            glm::vec4 v3 = {}){
        object_parameter.custom_vec0 = v0;
        object_parameter.custom_vec1 = v1;
        object_parameter.custom_vec2 = v2;
        object_parameter.custom_vec3 = v3;
    }
};

//-----------------------
//  struct mesh tree for top accel strucre
//-----------------------

//mesh tree ...
//blsa_meshsの中身がgeometryが
//全部同じmeshという想定の欠陥設計，許して
//custom_instance_idで参照vertex bufferを変えるので
//一つのBLASに一種類のメッシュは仕方ないのでは
struct instanceMeshs{
    std::vector<uint32_t> blas_meshs;
    glm::f32mat3x4 transform;
    uint32_t    hit_group_idx;
    uint32_t    mesh_idx;
};

struct meshTree{
    std::vector<uniquePolygonMesh> polygon_meshs;
    uniqueBufferVma object_parameters;
    std::vector<instanceMeshs> tree;
    
public:

    void updateObjectParams(vulkanEngine* ve){

        std::vector<uniquePolygonMesh::objectParameter> tmp;

        for(auto& pm: polygon_meshs){
            tmp.push_back(pm.object_parameter);
        }
        
        object_parameters = uniqueBufferVma(ve,
                                    sizeof(uniquePolygonMesh::objectParameter)*tmp.size(),
                                    tmp.data(),
                                    vk::BufferUsageFlagBits::eStorageBuffer
                                    |vk::BufferUsageFlagBits::eShaderDeviceAddress,
                                    VMA_MEMORY_USAGE_GPU_ONLY);
    }
};