#pragma once
#include "vulkanEngine.hpp"
#include "memoryVma.hpp"
#include "rtShader.hpp"
#include "rtPipeline.hpp"

struct sbtInfo{
    uint32_t group_count;
    uint32_t additional_data_byte;
    void*    additional_data;//all hit group have same data
};

struct uniqueRtShaderBindingTable{
    vulkanEngine *ve;

    //rt pipeline
    uniqueRtPipeline* rt_pipeline;
    
    //shaders
    uniqueRtShaders* rt_shaders;
    
    //shader binding table buffer
    uniqueBufferVma sbt_raygen_groups;
    uniqueBufferVma sbt_miss_groups;
    uniqueBufferVma sbt_hit_groups;
    
    //sbt region
    vk::StridedDeviceAddressRegionKHR raygen_region{};
    vk::StridedDeviceAddressRegionKHR miss_region{};
    vk::StridedDeviceAddressRegionKHR hit_region{};
    
    //sbt props
    uint32_t handle_size;   
    uint32_t handle_alignment;
    uint32_t base_alignment;    
    
    //handle strage
    std::vector<uint8_t> handle_storage;
    
    uniqueRtShaderBindingTable(){}
    uniqueRtShaderBindingTable(vulkanEngine* ve, 
                            uniqueRtPipeline* pipeline,
                            uniqueRtShaders* shaders)
                            :ve(ve),
                            rt_pipeline(pipeline),
                            rt_shaders(shaders){

        auto rt_pipeline_properties = getRayTracingProps();
        handle_size         = rt_pipeline_properties.shaderGroupHandleSize;
        handle_alignment    = rt_pipeline_properties.shaderGroupHandleAlignment;
        base_alignment      = rt_pipeline_properties.shaderGroupBaseAlignment;

        //get all handles
        handle_storage.resize(shaders->max_group_size*handle_size);
        auto result = ve->device->getRayTracingShaderGroupHandlesKHR(
                                            rt_pipeline->getPipeline(), 
                                            0,                      //first group
                                            shaders->max_group_size,//group count
                                            handle_storage.size(), 
                                            handle_storage.data());
        if (result != vk::Result::eSuccess) {
            exitStr("Failed to get ray tracing shader group handles.\n");
        }
    }
    
    auto& addRaygenGroup(sbtInfo sbt_info){

        raygen_region.setStride(recodeStride(sbt_info))
                    .setSize(raygen_region.stride); //special rule
        
        uint32_t raygen_handle_head_idx = 0;

        copySbtData2Buffer(raygen_region, 
                        sbt_raygen_groups,
                        sbt_info, 
                        raygen_handle_head_idx);

        return *this;
    }

    auto& addMissGroup(sbtInfo sbt_info){
        
        auto raygen_group_count = rt_shaders->_shader_group_info.raygen_group_count;
        auto miss_group_count = rt_shaders->_shader_group_info.miss_group_count;

        miss_region.setStride(recodeStride(sbt_info))
                    .setSize(miss_region.stride*miss_group_count); 
        
        uint32_t miss_handle_head_idx = miss_group_count;

        copySbtData2Buffer(miss_region, 
                        sbt_miss_groups,
                        sbt_info, 
                        miss_handle_head_idx);

        return *this;
    }

    auto& addHitGroup(sbtInfo sbt_info){

        auto raygen_group_count = rt_shaders->_shader_group_info.raygen_group_count;
        auto miss_group_count = rt_shaders->_shader_group_info.miss_group_count;
        auto hit_group_count = rt_shaders->_shader_group_info.hit_group_count;

        hit_region.setStride(recodeStride(sbt_info))
                    .setSize(hit_region.stride*hit_group_count); 
        
        uint32_t hit_handle_head_idx 
                    = raygen_group_count + miss_group_count;

        copySbtData2Buffer(hit_region, 
                        sbt_hit_groups,
                        sbt_info, 
                        hit_handle_head_idx);
        return *this;
    }

private:
    uint32_t alignUp(uint32_t size, uint32_t alignment) {
        return (size + alignment - 1) & ~(alignment - 1); 
    }

    uint32_t recodeStride(sbtInfo sbt_info){
        return alignUp(handle_size+sbt_info.additional_data_byte
                                    ,handle_alignment);
    }
    
    void copySbtData2Buffer(vk::StridedDeviceAddressRegionKHR& region, 
                            uniqueBufferVma& buffer,
                            sbtInfo& sbt_info,
                            uint32_t handle_head_idx){


        std::vector<uint8_t> tmp_recode_buf(region.size);

        for(int i=0; i<sbt_info.group_count; i++){
            //copy handle
            std::memcpy(tmp_recode_buf.data()+i*region.stride,  //dst
                        handle_storage.data()+(i+handle_head_idx)*handle_size,  //src
                        handle_size);    //size
            
            //copy additional data
            std::memcpy(tmp_recode_buf.data()+i*region.stride+handle_size,  //dst
                        sbt_info.additional_data,           //src
                        sbt_info.additional_data_byte);    //size
        }
        
        buffer = uniqueBufferVma(
                ve,
                region.size,
                tmp_recode_buf.data(),
                vk::BufferUsageFlagBits::eShaderBindingTableKHR|
                vk::BufferUsageFlagBits::eTransferSrc|
                vk::BufferUsageFlagBits::eShaderDeviceAddress,
                VMA_MEMORY_USAGE_GPU_ONLY,
                base_alignment);

        region.setDeviceAddress(buffer.address);
    }


    //utility
    vk::PhysicalDeviceRayTracingPipelinePropertiesKHR getRayTracingProps() {
        auto device_properties 
            = ve->physical_device.getProperties2<vk::PhysicalDeviceProperties2,
                            vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();

        return device_properties.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
    }
};