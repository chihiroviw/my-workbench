#pragma once
#include <stdio.h>
#include <cstring>

#include "vulkanEngine.hpp"
#include "memoryVma.hpp"
#include "swapchain.hpp"
#include "utility/glfw/vk_window.h"
#include "utility/error/err.hpp"


//------------------------
//  test object
//------------------------
#include <glm/glm.hpp>

struct alignas(32) Vertex{
    glm::f32vec3 v;
};

struct alignas(32) VertexC{
    glm::f32vec3 v;
    glm::f32vec3 c;
};

struct alignas(32) VertexCN{
    glm::f32vec3 v;
    glm::f32vec3 c;
    glm::f32vec3 n;
};

struct alignas(64) sceneData {
    glm::f32vec3 rectCenter;
};


#define VERTEX VertexC

std::vector<VERTEX> vertices = {
    VERTEX{{-0.5, -0.5 , 0.0},{1.0,0.0,0.0}},//r g b
    VERTEX{{ 0.5,  0.5 , 0.0},{0.0,0.0,1.0}},
    VERTEX{{-0.5,  0.5 , 0.0},{1.0,0.0,0.0}},
    VERTEX{{ 0.5, -0.5 , 0.0},{0.0,0.0,1.0}},
};

std::vector<VERTEX> vertices1 = {
    VERTEX{{-0.0, -0.7 , 0.0},{0.0,1.0,0.0}},
    VERTEX{{ 0.7,  0.7 , 0.0},{1.0,0.0,0.0}},
    VERTEX{{-0.7,  0.7 , 0.0},{0.0,0.0,1.0}},
};

std::vector<uint32_t> indices = {0,1,2,1,0,3};

sceneData scene_data = { { 0.3, -0.2 ,0.0} };






//----------------------------
//  main
//----------------------------
class rasterVulkanEngine : public vulkanEngine{

protected:         
    /*time*/
    float time=0;

    /*vertex buffer*/
    uniqueBufferVma vertex_buffer[2];
       
    /*vertex binding+attribution*/
    vk::VertexInputBindingDescription vertex_binding_description[1];
    vk::VertexInputAttributeDescription vertex_attrib_description[2];
    
    /*index buffer*/
    uniqueBufferVma index_buffer;
    
    /*uniform buffer*/
    uniqueBufferVma uniform_buffer;

    vk::UniqueDescriptorPool desc_pool;
    vk::UniqueDescriptorSetLayout desc_set_layout; 
    std::vector<vk::UniqueDescriptorSet> desc_sets;
    
    /*swapchain*/
    uniqueSwapchain swapchain;
    
    /*semaphore + fence  for window drawing*/
    //aquireNextImage->(present_comlete)->submit->(render_comlete)->present->(submit{Fence})
    vk::UniqueSemaphore next_img_ready_semaphore; //set to aquireNextImage
    vk::UniqueSemaphore render_complete_semaphore; //set to queue.submit
    vk::UniqueFence submit_wait_fence;
    
    /*image*/
    vk::UniqueImage image;
    vk::UniqueDeviceMemory img_mem;
    vk::UniqueImageView imgview; 
    vk::UniqueFramebuffer frame_buf;
    
    /*render pass*/
    vk::UniqueRenderPass render_pass;
    vk::UniquePipelineLayout pipeline_layout;
    vk::UniquePipeline pipeline;
    
    /*shader module*/
    vk::UniqueShaderModule vert_shader, frag_shader;
    
public:
    //renderpass ->(shader ->) pipeline -> image+view -> framebuf
    rasterVulkanEngine(glfwWindow* w){

        /*initialize*/
        vulkanEngine::init(w);

        /*create vertex buffer*/
        createVertexBuffer(vertices,0); 
        createVertexBuffer(vertices1,1); 
        createVertexDescription();//binding+atribute

        /*create index buffer*/
        createIndexBuffer();

        /*create uniform buffer*/
        createUniformBuffer();
        createUniformBufDescription();
        
        /*render pass + graphics pipeline*/
        createRenderPass(); 
        createGraphicsPipeline();         
        
        /*create + swapchain (+view +framebuffer)*/
        //surface -> swapchain -> images + views ->(renderpass->) framebuf
        createSwapchain();

        /*image + framebuffer*/
        //createImage();
        //allocImgMem();
        //device->bindImageMemory(image.get(),img_mem.get(),0);
        //createImageView();
        //createFrameBuffer();
    }

    void drawFrame(){
        cmd_bufs[0]->reset();

        vk::CommandBufferBeginInfo cmd_begin_info;
        cmd_bufs[0]->begin(cmd_begin_info);
        
        vk::ClearValue clear_val;
        clear_val.color.setFloat32({0.0f,0.0f,0.0f,1.0f});

        vk::RenderPassBeginInfo renderpass_begin_info;
        renderpass_begin_info.setRenderPass(render_pass.get())
                            .setFramebuffer(swapchain.nextRawFramebuffer())
                            .setRenderArea(vk::Rect2D({ 0,0 }, {window->w(), window->h()}))
                            .setClearValueCount(1)
                            .setPClearValues(&clear_val);

        cmd_bufs[0]->beginRenderPass(renderpass_begin_info, vk::SubpassContents::eInline);

        //subpass[0]
        cmd_bufs[0]->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());
        cmd_bufs[0]->bindVertexBuffers(0,{vertex_buffer[0].buffer},{0});
        //cmd_bufs[0]->bindIndexBuffer(index_buffer.rawBuffer(),0,vk::IndexType::eUint32);
        //printf("ok1\n");
        cmd_bufs[0]->bindIndexBuffer(index_buffer.buffer,0,vk::IndexType::eUint32);
        //printf("ok2\n");
        cmd_bufs[0]->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout.get(), 0, { desc_sets[0].get() }, {});

        //update uniform buf + draw
        scene_data.rectCenter = glm::f32vec3{ 0.3f * cosf(time), 0.3f * sinf(time),0.0 };
        uniform_buffer.copyDataToMemory(&scene_data,sizeof(scene_data));
        
        //draw
        cmd_bufs[0]->drawIndexed((uint32_t)indices.size(),1,0,0,0);

        cmd_bufs[0]->bindVertexBuffers(0,{vertex_buffer[1].buffer},{0});
        cmd_bufs[0]->drawIndexed(3,1,0,0,0);

        cmd_bufs[0]->endRenderPass();  
        
        cmd_bufs[0]->end(); 

        /*submit info*/
        vk::SubmitInfo submit_info;
        auto raw_cmd_bufs = {cmd_bufs[0].get(), cmd_bufs[1].get()};
        submit_info .setCommandBufferCount(1)
                    .setPCommandBuffers(raw_cmd_bufs.begin());

        /*wait semaphore + signal semaphore*/
        vk::PipelineStageFlags renderwait_stage[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submit_info .setWaitSemaphoreCount(1)
                    .setPWaitSemaphores(&next_img_ready_semaphore.get())
                    .setPWaitDstStageMask(renderwait_stage)
                    .setSignalSemaphoreCount(1)
                    .setPSignalSemaphores(&render_complete_semaphore.get());
        
        queue.submit({submit_info},submit_wait_fence.get());
    }
    
    void run() {
        //fence
        vk::FenceCreateInfo fence_create_info;
        submit_wait_fence = device->createFenceUnique(fence_create_info);

        //semaphore
        vk::SemaphoreCreateInfo semaphore_create_info;
        next_img_ready_semaphore = device->createSemaphoreUnique(semaphore_create_info);
        render_complete_semaphore = device->createSemaphoreUnique(semaphore_create_info);
        
        while(window->pollEvents()){
            window->printFps();
            time += (float)0.01;
            
            //get next swapchain image
            swapchain.acquireNextImage(next_img_ready_semaphore);

            //draw
            drawFrame();
            
            //presentation
            queue.presentKHR(
                vk::PresentInfoKHR()
                    .setSwapchainCount(1)
                    .setPSwapchains(&(swapchain.swapchain.get()))
                    .setPImageIndices(&swapchain.img_idx)
                    .setWaitSemaphoreCount(1)
                    .setPWaitSemaphores(&render_complete_semaphore.get())
            );

            //wait for rendering end
            device->waitForFences({submit_wait_fence.get()},VK_TRUE, UINT64_MAX);
            device->resetFences({submit_wait_fence.get()});
        }

        queue.waitIdle();   
    }
    
    void createImage(){
        vk::ImageCreateInfo img_create_info;
        img_create_info .setImageType(vk::ImageType::e2D)
                        .setExtent(vk::Extent3D(window->w(),window->h(),1))
                        .setMipLevels(1)
                        .setArrayLayers(1)
                        .setFormat(vk::Format::eR8G8B8A8Unorm)
                        .setTiling(vk::ImageTiling::eLinear)
                        .setInitialLayout(vk::ImageLayout::eUndefined)
                        .setUsage(vk::ImageUsageFlagBits::eColorAttachment)
                        .setSharingMode(vk::SharingMode::eExclusive)
                        .setSamples(vk::SampleCountFlagBits::e1);
        
        image = device->createImageUnique(img_create_info);
    }
    
    void allocImgMem(){
        vk::PhysicalDeviceMemoryProperties mem_props = physical_device.getMemoryProperties();
        vk::MemoryRequirements img_mem_req = device->getImageMemoryRequirements(image.get());
        
        vk::MemoryAllocateInfo img_mem_alloc_info;
        img_mem_alloc_info.setAllocationSize(img_mem_req.size);
        
        bool suitable_memory_type_found = false;
        for(uint32_t i=0; i< mem_props.memoryTypeCount; i++){
            if(img_mem_req.memoryTypeBits & (1<<i)){
                img_mem_alloc_info.memoryTypeIndex = i;
                suitable_memory_type_found = true;
                break;
            }
        }
        
        if(!suitable_memory_type_found)exitStr("alloc img memory error\n"); 
        img_mem = device->allocateMemoryUnique(img_mem_alloc_info);
    }
    
    void createImageView(){
        vk::ImageViewCreateInfo imgview_create_info;
        imgview_create_info.image = image.get();
        imgview_create_info.viewType = vk::ImageViewType::e2D;
        imgview_create_info.format = vk::Format::eR8G8B8A8Unorm;
        imgview_create_info.components.r = vk::ComponentSwizzle::eIdentity;
        imgview_create_info.components.g = vk::ComponentSwizzle::eIdentity;
        imgview_create_info.components.b = vk::ComponentSwizzle::eIdentity;
        imgview_create_info.components.a = vk::ComponentSwizzle::eIdentity;
        imgview_create_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        imgview_create_info.subresourceRange.baseMipLevel = 0;
        imgview_create_info.subresourceRange.levelCount = 1;
        imgview_create_info.subresourceRange.baseArrayLayer = 0;
        imgview_create_info.subresourceRange.layerCount = 1;

        imgview = device->createImageViewUnique(imgview_create_info);
    }
    
    void createFrameBuffer(){
        vk::ImageView framebuf_attachments[1];
        framebuf_attachments[0] = imgview.get();
        
        vk::FramebufferCreateInfo framebuf_create_info;
        framebuf_create_info.width = window->w();
        framebuf_create_info.height = window->h();
        framebuf_create_info.layers = 1;
        framebuf_create_info.renderPass = render_pass.get();
        framebuf_create_info.attachmentCount = 1;
        framebuf_create_info.pAttachments = framebuf_attachments;

        frame_buf = device->createFramebufferUnique(framebuf_create_info);
    }
    
    void createRenderPass(){
        vk::AttachmentDescription attachments[1];

        attachments[0].format = surface_format.format;//swapchain_format == surface_format
        attachments[0].samples = vk::SampleCountFlagBits::e1;
        attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
        attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        attachments[0].initialLayout = vk::ImageLayout::eUndefined;
        attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference subpass0_attachment_refs[1];
        subpass0_attachment_refs[0].attachment = 0;
        subpass0_attachment_refs[0].layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpasses[1];
        subpasses[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpasses[0].colorAttachmentCount = 1;
        subpasses[0].pColorAttachments = subpass0_attachment_refs;

        vk::RenderPassCreateInfo renderpass_create_info; 
        renderpass_create_info.attachmentCount = 1;
        renderpass_create_info.pAttachments = attachments;
        renderpass_create_info.subpassCount = 1;
        renderpass_create_info.pSubpasses = subpasses;
        renderpass_create_info.dependencyCount = 0;
        renderpass_create_info.pDependencies = nullptr;

        render_pass = device->createRenderPassUnique(renderpass_create_info);
    }
    
    void createGraphicsPipeline(){
 
        /*shader module*/
        vert_shader = createShaderModule("./src/shader/shader.vert.spv");
        frag_shader = createShaderModule("./src/shader/shader.frag.spv");


        vk::Viewport viewports[1];
        viewports[0].setX(0.0).setY(0.0)
                    .setMinDepth(0.0).setMinDepth(1.0)
                    .setWidth((float)window->w())
                    .setHeight((float)window->h());

        vk::Rect2D scissors[1];
        scissors[0].offset = vk::Offset2D{0,0};
        scissors[0].extent = vk::Extent2D{window->w(),window->h()};

        vk::PipelineViewportStateCreateInfo viewport_state;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = viewports;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = scissors;

        vk::PipelineVertexInputStateCreateInfo vertex_input_info;
        vertex_input_info.vertexBindingDescriptionCount = 1;
        vertex_input_info.pVertexBindingDescriptions = vertex_binding_description;
        vertex_input_info.vertexAttributeDescriptionCount = 2;
        vertex_input_info.pVertexAttributeDescriptions = vertex_attrib_description;

        vk::PipelineInputAssemblyStateCreateInfo input_assembly;
        input_assembly.topology = vk::PrimitiveTopology::eTriangleList;
        input_assembly.primitiveRestartEnable = false;

        vk::PipelineRasterizationStateCreateInfo rasterizer;
        rasterizer.depthClampEnable = false;
        rasterizer.rasterizerDiscardEnable = false;
        rasterizer.polygonMode = vk::PolygonMode::eFill;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = vk::CullModeFlagBits::eBack;
        rasterizer.frontFace = vk::FrontFace::eClockwise;
        rasterizer.depthBiasEnable = false;

        vk::PipelineMultisampleStateCreateInfo multi_sample;
        multi_sample.sampleShadingEnable = false;
        multi_sample.rasterizationSamples = vk::SampleCountFlagBits::e1;

        vk::PipelineColorBlendAttachmentState blendattachment[1];
        blendattachment[0].colorWriteMask =
            vk::ColorComponentFlagBits::eA |
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB;
        blendattachment[0].blendEnable = false;

        vk::PipelineColorBlendStateCreateInfo blend;
        blend.logicOpEnable = false;
        blend.attachmentCount = 1;
        blend.pAttachments = blendattachment;

        auto pipeline_desc_set_layouts = { desc_set_layout.get() };
        vk::PipelineLayoutCreateInfo layout_create_info;
        layout_create_info.setLayoutCount = (uint32_t)pipeline_desc_set_layouts.size();
        layout_create_info.pSetLayouts = pipeline_desc_set_layouts.begin();

        pipeline_layout = device->createPipelineLayoutUnique(layout_create_info);

        vk::PipelineShaderStageCreateInfo shader_stage[2];
        shader_stage[0].stage = vk::ShaderStageFlagBits::eVertex;
        shader_stage[0].module = vert_shader.get();
        shader_stage[0].pName = "main";
        shader_stage[1].stage = vk::ShaderStageFlagBits::eFragment;
        shader_stage[1].module = frag_shader.get();
        shader_stage[1].pName = "main";
        
        vk::GraphicsPipelineCreateInfo pipeline_create_info;
        pipeline_create_info.pViewportState = &viewport_state;
        pipeline_create_info.pVertexInputState = &vertex_input_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly;
        pipeline_create_info.pRasterizationState = &rasterizer;
        pipeline_create_info.pMultisampleState = &multi_sample;
        pipeline_create_info.pColorBlendState = &blend;
        pipeline_create_info.layout = pipeline_layout.get();
        pipeline_create_info.stageCount = 0;
        pipeline_create_info.pStages = nullptr;
        pipeline_create_info.renderPass = render_pass.get(); 
        pipeline_create_info.subpass = 0;
        pipeline_create_info.stageCount = 2;
        pipeline_create_info.pStages = shader_stage;

        pipeline = device->createGraphicsPipelineUnique(nullptr, pipeline_create_info).value;
    }
    
    void createSwapchain(){
        swapchain = uniqueSwapchain(this,render_pass);
    }    

    void createVertexBuffer(std::vector<VERTEX> vs, size_t dst_index){
        size_t size = sizeof(VERTEX)*vs.size();

        vertex_buffer[dst_index] = uniqueBufferVma(this,size,vs.data(),
                                        vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eTransferDst,
                                        VMA_MEMORY_USAGE_GPU_ONLY);
    }
    
    
    void createVertexDescription(){
        vertex_binding_description[0].setBinding(0)
                                    .setStride(sizeof(VERTEX))
                                    .setInputRate(vk::VertexInputRate::eVertex);
        
        vertex_attrib_description[0].setBinding(0)
                                    .setLocation(0)
                                    .setFormat(vk::Format::eR32G32B32Sfloat)
                                    .setOffset(offsetof(VERTEX,v));
         
        vertex_attrib_description[1].setBinding(0)
                                    .setLocation(1)
                                    .setFormat(vk::Format::eR32G32B32Sfloat)
                                    .setOffset(offsetof(VERTEX,c));
    }

    void createIndexBuffer(){
        size_t size  = sizeof(uint32_t) * indices.size();
        index_buffer = uniqueBufferVma(
                        this,size,indices.data(),
                        vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst,
                        VMA_MEMORY_USAGE_GPU_ONLY);
    }
    
    void createUniformBuffer(){
        size_t size  = sizeof(sceneData);
        uniform_buffer = uniqueBufferVma(
                            this,size,&scene_data,
                            vk::BufferUsageFlagBits::eUniformBuffer,
                            VMA_MEMORY_USAGE_CPU_ONLY);  
    }

    void createUniformBufDescription(){
        /*create desc set layout*/
        ////set=0 binding=0
        vk::DescriptorSetLayoutBinding desc_set_layout_bindings[1]; 
        desc_set_layout_bindings[0].setBinding(0)
                                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                .setDescriptorCount(1)
                                .setStageFlags(vk::ShaderStageFlagBits::eVertex);

        ////create "one descriptor set layout" 
        //  from array of "descriptor binding infos"
        vk::DescriptorSetLayoutCreateInfo desc_set_layout_create_info; 
        desc_set_layout_create_info.setBindingCount(1)
                                .setPBindings(desc_set_layout_bindings);

        desc_set_layout = device->createDescriptorSetLayoutUnique(desc_set_layout_create_info);
        
        /*create desc pool*/
        vk::DescriptorPoolSize desc_pool_size[1];
        desc_pool_size[0].type = vk::DescriptorType::eUniformBuffer;
        desc_pool_size[0].descriptorCount = 1;

        vk::DescriptorPoolCreateInfo desc_pool_create_info;
        desc_pool_create_info.setPoolSizeCount(1)
                            .setPoolSizes(desc_pool_size)
                            .setMaxSets(1)
                            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

        desc_pool = device->createDescriptorPoolUnique(desc_pool_create_info);
        
        /*create desc set*/
        vk::DescriptorSetAllocateInfo desc_set_alloc_info;

        auto desc_set_layouts = { desc_set_layout.get() }; 
        desc_set_alloc_info.setDescriptorPool(desc_pool.get())
                        .setDescriptorSetCount((uint32_t)desc_set_layouts.size())
                        .setPSetLayouts(desc_set_layouts.begin());

        desc_sets = device->allocateDescriptorSetsUnique(desc_set_alloc_info); 
        
        /*update descriptor set*/
        //set=0 binding=0 <-> buffer
        vk::DescriptorBufferInfo desc_buf_infos[1];
        desc_buf_infos[0].setBuffer(uniform_buffer.buffer)
                        .setOffset(0)
                        .setRange(sizeof(sceneData));

        vk::WriteDescriptorSet write_desc_sets[1];
        write_desc_sets[0].setDstSet(desc_sets[0].get())//set=0 in desc_sets[0]
                        .setDstBinding(0)//set=0 binding=0
                        .setDstArrayElement(0).setDescriptorCount(1) //from 0 to 1
                        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                        .setPBufferInfo(&desc_buf_infos[0]);

        device->updateDescriptorSets(write_desc_sets, {});
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
    }
    virtual void addDeviceFeatures() override{}
};