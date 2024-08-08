#pragma once
#include "vulkanEngine.hpp"
#include "memoryVma.hpp"

struct rtShaderInfo{
    const char* filename;
    vk::ShaderStageFlagBits stage;
};

struct rtShaderGroupInfo{
    uint32_t raygen_group_count;
    uint32_t miss_group_count;
    uint32_t hit_group_count;
    //uint32_t call_group_count;//unused
};

struct uniqueRtShaders{
    vulkanEngine* ve;

    //shader module
    std::vector<vk::UniqueShaderModule> shader_modules;
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
    
    //shader group 
    rtShaderGroupInfo _shader_group_info;
    uint32_t max_group_size;
    std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shader_groups;
    
    
    uniqueRtShaders(){}
    uniqueRtShaders(vulkanEngine* ve, 
                    std::vector<rtShaderInfo>& shader_infos,
                    rtShaderGroupInfo shader_group_info,
                    std::vector<vk::RayTracingShaderGroupCreateInfoKHR> sg)
                    :ve(ve),
                    _shader_group_info(shader_group_info),
                    shader_groups(sg){
        
        shader_modules.reserve(shader_infos.size());        
        shader_stages.reserve(shader_infos.size());
        
        max_group_size = shader_group_info.raygen_group_count
                        +shader_group_info.miss_group_count
                        +shader_group_info.hit_group_count;
        
        for(auto& shader: shader_infos){
            //add shader module
            shader_modules.push_back(ve->createShaderModule((char*)shader.filename));

            //set shader_stage_info
            shader_stages.push_back(vk::PipelineShaderStageCreateInfo()
                                .setStage(shader.stage)
                                .setModule(*shader_modules.back())
                                .setPName("main"));
        }
    }

private:
    vk::UniqueShaderModule createShaderModule(char *file_name){
        /*read file*/
        if(!std::filesystem::exists(file_name)){
            printf("file doesn't exit: %s\n",file_name);
            exit(-1);
        }
        size_t spvfile_size = std::filesystem::file_size(file_name);
        std::ifstream spv_file(file_name, std::ios_base::binary);
        std::vector<char> spvfile_data(spvfile_size);
        spv_file.read(spvfile_data.data(), spvfile_size);
        
        /*create shader module*/
        vk::ShaderModuleCreateInfo shader_create_info;
        shader_create_info.setCodeSize(spvfile_size) 
                        .setPCode(reinterpret_cast<const uint32_t*>(spvfile_data.data()));

        return ve->device->createShaderModuleUnique(shader_create_info);
    }
};

