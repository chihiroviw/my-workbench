#pragma once
#include "vulkanEngine.hpp"
#include "descriptorSets.hpp"
#include "rtShader.hpp"

struct uniqueRtPipeline{
    vulkanEngine* ve;

    //raytracing pipeline
    vk::UniquePipelineLayout rt_pipeline_layout;
    vk::UniquePipeline rt_pipeline;
    
    uniqueRtPipeline(){}

    uniqueRtPipeline(vulkanEngine* ve,
                    uniqueDescriptorSets& descriptor_sets,
                    uniqueRtShaders& rt_shaders)
                    :ve(ve){

        // Create pipeline layout
        vk::PipelineLayoutCreateInfo layout_create_info;
        layout_create_info.setSetLayouts(descriptor_sets.desc_set_layouts);
        rt_pipeline_layout = ve->device->createPipelineLayoutUnique(layout_create_info);

        // Create pipeline
        vk::RayTracingPipelineCreateInfoKHR pipeline_create_info;
        pipeline_create_info.setLayout(*rt_pipeline_layout)
                        .setStages(rt_shaders.shader_stages)
                        .setGroups(rt_shaders.shader_groups)
                        .setMaxPipelineRayRecursionDepth(1);
        auto result = ve->device->createRayTracingPipelineKHRUnique(nullptr, 
                                                                    nullptr, 
                                                                    pipeline_create_info);

        if (result.result != vk::Result::eSuccess) {
            exitStr("Failed to create ray tracing pipeline.\n");
        }

        rt_pipeline = std::move(result.value);
    }
    
    
    auto getPipeline(){return rt_pipeline.get();}
    auto getLayout(){return rt_pipeline_layout.get();}

};