#pragma once
#include <stdio.h>
#include <cstring>

#include "vulkanEngine.hpp"
#include "memoryVma.hpp"
#include "swapchain.hpp"
#include "accelerationStructre.hpp"
#include "vertexIndexBuffer.hpp"
#include "descriptorSets.hpp"
#include "rtPipeline.hpp"
#include "rtShader.hpp"
#include "rtShaderBindingTable.hpp"

#include "utility/glfw/vk_window.h"
#include "rtCamera.hpp"

//test 
#include "rtObjTest.hpp"


class rtVulkanEngine : public vulkanEngine{
public:
    //camera
    rtCamera camera;

    //mesh buffer
    meshTree mesh_tree;
    
    //accel structre
    uniqueTopAccelStruct tlas;
    
    //shader module etc...
    uniqueRtShaders rt_shaders;
    
    //shader binding table
    uniqueRtShaderBindingTable sbt;
    
    //desctiptor
    uniqueDescriptorSets descriptor_sets;
    
    //raytracing pipeline
    uniqueRtPipeline rt_pipeline;
    
    /*swapchain*/
    uniqueSwapchain swapchain;
    
public:
    rtVulkanEngine(glfwWindow* w){
        /*initialize*/
        vulkanEngine::init(w);       
        
        /*create mesh*/
        //mesh_tree = prepareScene(this);
        mesh_tree = prepareSceneTest0(this);

        /*create accel struct*/
        //createAS();
        tlas = createTlasBlas(this,mesh_tree);

        /*shader*/
        prepareShaders();

        /*descriptor*/
        createDescriptorSets();
        
        /*create raytracing pipeline*/
        rt_pipeline = uniqueRtPipeline(this, descriptor_sets, rt_shaders);
        
        /*create shader binding table*/
        createShaderBindingTable();

        /*create rt swapchain*/
        swapchain = uniqueSwapchain(this);
        
        /*init camera*/
        camera = rtCamera(this, window->w(), window->h(),
                        {278,278,-1200},//lookfrom
                        {278,278,0});  //look at
    }
    
    void run(){
        //fence (sync cpu and gpu)
        vk::FenceCreateInfo fence_create_info;
        auto submit_wait_fence = device->createFenceUnique(fence_create_info);

        //semaphore (sync gpu command and gpu command)
        vk::SemaphoreCreateInfo semaphore_create_info;
        auto next_img_ready_semaphore = device->createSemaphoreUnique(semaphore_create_info);
        auto render_complete_semaphore = device->createSemaphoreUnique(semaphore_create_info);
        
        //update descriptor (uniform variable)
        descriptor_sets .addUpdateInfo(0,0,camera.buffer)
                        .addUpdateInfo(0,1,mesh_tree.object_parameters)
                        .addUpdateInfo(1,0,tlas.top_as);


        window->loop([&](){
            //write date to uniform buffer
            camera.updateBuffer();
            
            //get next swapchain image 
            swapchain.acquireNextImage(next_img_ready_semaphore);

            //update descriptor set
            descriptor_sets .addUpdateInfo( 2, 0,//set, binding
                                swapchain.nextRawImageView(),
                                vk::ImageLayout::eGeneral)
                            .update();

            //draw
            drawFrame(next_img_ready_semaphore, //wait semaphore
                    render_complete_semaphore,  //signal semaphore
                    submit_wait_fence);         //signal fence

            //presentation
            swapchain.presentation(render_complete_semaphore);//wait semaphore
            
            //cpu blank time, update 
            camera.update(window->getKeyResponse(),window->getMouseOffset());

            //wait for rendering end
            device->waitForFences({*submit_wait_fence}, VK_TRUE, UINT64_MAX);
            device->resetFences({*submit_wait_fence});
        });
    
        queue.waitIdle();   
    }
    
    void drawFrame(vk::UniqueSemaphore& wait_semaphore,
                   vk::UniqueSemaphore& signal_semaphore,
                   vk::UniqueFence& signal_fence){

        /*record command buffer*/
        recordCommands(cmd_bufs[0]);
        
        /*submit*/
        vk::PipelineStageFlags wait_stage{vk::PipelineStageFlagBits::eTopOfPipe};
        auto raw_cmd_bufs = {*cmd_bufs[0]};

        vk::SubmitInfo submit_info{}; 
        submit_info .setCommandBufferCount(1)
                    .setPCommandBuffers(raw_cmd_bufs.begin())
                    .setWaitSemaphoreCount(1)
                    .setPWaitSemaphores(&wait_semaphore.get())
                    .setWaitDstStageMask(wait_stage)
                    .setSignalSemaphoreCount(1)
                    .setPSignalSemaphores(&signal_semaphore.get());

        queue.submit({submit_info},signal_fence.get());
    } 
    
    void recordCommands(vk::UniqueCommandBuffer& command_buffer){
        // Begin
        command_buffer->begin(vk::CommandBufferBeginInfo{});

        //change image layout to general
        cmdChangeImageLayout(*command_buffer, 
                        swapchain.nextImage(),
                        vk::ImageLayout::ePresentSrcKHR,
                        vk::ImageLayout::eGeneral);

        // Bind pipeline
        command_buffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR
                                    ,rt_pipeline.getPipeline());

        // Bind desc set
        // you need to bind every draw loop
        command_buffer->bindDescriptorSets(
            vk::PipelineBindPoint::eRayTracingKHR,  // pipelineBindPoint
            rt_pipeline.getLayout(),                // layout
            0,                                      // firstSet
            descriptor_sets.desc_sets,              // descSets
            nullptr                                 // dynamicOffsets
        );

        // Trace rays
        command_buffer->traceRaysKHR(   
            sbt.raygen_region,          //raygen
            sbt.miss_region,            // miss
            sbt.hit_region,             // hit
            {},                         // callable
            window->w(), window->h(), 1 // width, height, depth
        );

        //change image layout to present src
        cmdChangeImageLayout(*command_buffer, 
                        swapchain.nextImage(),  
                        vk::ImageLayout::eGeneral,
                        vk::ImageLayout::ePresentSrcKHR);

        // End
        command_buffer->end();
    }
    
    void prepareShaders(){
        //create (shader modules + shade stages + group)
        std::vector<rtShaderInfo> shader_infos = {
            {"./src/shader/rt0/raygen.rgen.spv",        //index 0
                vk::ShaderStageFlagBits::eRaygenKHR},
            {"./src/shader/rt0/miss.rmiss.spv",         //index 1
                vk::ShaderStageFlagBits::eMissKHR},
            {"./src/shader/rt0/phoneShading.rchit.spv",    //index 2
                vk::ShaderStageFlagBits::eClosestHitKHR},
            {"./src/shader/rt0/refract.rchit.spv",    //index 3
                vk::ShaderStageFlagBits::eClosestHitKHR},
        };
        
        rtShaderGroupInfo shader_group_info{
            1,  //raygen group count
            1,  //miss group count
            2   //hit group count
        };
        
        std::vector<vk::RayTracingShaderGroupCreateInfoKHR> 
            shader_groups = {
                vk::RayTracingShaderGroupCreateInfoKHR()
                    .setType(vk::RayTracingShaderGroupTypeKHR::eGeneral)
                    .setGeneralShader(0),

                vk::RayTracingShaderGroupCreateInfoKHR()
                    .setType(vk::RayTracingShaderGroupTypeKHR::eGeneral)
                    .setGeneralShader(1),

                vk::RayTracingShaderGroupCreateInfoKHR()
                    .setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup)
                    .setClosestHitShader(2),

                vk::RayTracingShaderGroupCreateInfoKHR()
                    .setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup)
                    .setClosestHitShader(3),
        };

        rt_shaders = uniqueRtShaders(this,
                                    shader_infos,
                                    shader_group_info,
                                    shader_groups);
    }
   
    void createDescriptorSets(){
       
        std::vector<std::vector<vk::DescriptorSetLayoutBinding>> desc_sets_bindings = {
            {//set = 0
                {//set = 0, binding = 0
                    0,  //binging
                    vk::DescriptorType::eUniformBuffer,//desc type
                    1,  //descriptor count
                    vk::ShaderStageFlagBits::eRaygenKHR //shader stage
                },
                {//set = 0, binding = 1
                    1,  //binging
                    vk::DescriptorType::eStorageBuffer,//desc type
                    1,  //descriptor count
                    vk::ShaderStageFlagBits::eRaygenKHR //shader stage
                    |vk::ShaderStageFlagBits::eClosestHitKHR
                },
            },
            {//set = 1
                {//set = 1, binding = 0
                    0,  //binging
                    vk::DescriptorType::eAccelerationStructureKHR,//desc type
                    1,  //descriptor count
                    vk::ShaderStageFlagBits::eRaygenKHR //shader stage
                },
            },
            {//set = 2
                {//set = 2, binding = 0
                    0,  //binging
                    vk::DescriptorType::eStorageImage,
                    1,  //descriptor count
                    vk::ShaderStageFlagBits::eRaygenKHR
                },
            },
        };
        
        descriptor_sets 
            = uniqueDescriptorSets(this,std::move(desc_sets_bindings));
    }
    
    void createShaderBindingTable(){
        sbt = uniqueRtShaderBindingTable(this,&rt_pipeline,&rt_shaders);

        sbtInfo sbt_info_rgen{   
                            1,      //count
                            0,      //bytes
                            nullptr //data
                        };   

        sbtInfo sbt_info_miss{   
                            1,      //count
                            0,      //bytes
                            nullptr //data
                        };

        sbtInfo sbt_info_chit{   
                            2,      //count
                            0,      //bytes
                            nullptr //data
                        };

        sbt .addRaygenGroup(sbt_info_rgen)
            .addMissGroup(sbt_info_miss)
            .addHitGroup(sbt_info_chit);
    }
    
    

    
protected: 

    /*layer + extention + feature*/
    virtual void addInstanceLayers() override{}
    virtual void addInstanceExtentions() override{
        window->setInstanceExtention(&inst_required_extentions);
    }

    virtual void addDeviceLayers() override{}
    virtual void addDeviceExtentions() override{
        dev_required_extentions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        dev_required_extentions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
        dev_required_extentions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

        //for vk raytracing API
        dev_required_extentions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        dev_required_extentions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        dev_required_extentions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

        //for descriptor indexing
        dev_required_extentions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        
        //other
        dev_required_extentions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
        dev_required_extentions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        dev_required_extentions.push_back(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    }
    virtual void addDeviceFeatures() override{   
        //create structur chain
        static auto f0 = vk::PhysicalDeviceRayTracingPipelineFeaturesKHR{VK_TRUE};
        static auto f1 = vk::PhysicalDeviceAccelerationStructureFeaturesKHR{VK_TRUE};   f1.pNext = &f0;
        static auto f2 = vk::PhysicalDeviceBufferDeviceAddressFeatures{VK_TRUE};        f2.pNext = &f1;
        static auto f3 = vk::PhysicalDeviceDescriptorIndexingFeatures{VK_TRUE};         f3.pNext = &f2;
        static auto f4 = vk::PhysicalDeviceScalarBlockLayoutFeatures{VK_TRUE};          f4.pNext = &f3;

        //create info chain
        physical_device_features2.pNext = &f4;
        
        //enable features
        physical_device_features2.features.setShaderInt64(VK_TRUE);
    } 
};