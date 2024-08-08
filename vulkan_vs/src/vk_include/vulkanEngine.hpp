#pragma once
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <functional>

// vulkan hpp
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

// vma 
#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1002000 //vulkan 1.2
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vma/vk_mem_alloc.h>

// other utility
#include "utility/glfw/vk_window.h"
#include "utility/error/err.hpp"

// enable debug layer
#define VK_DEBUG_LAYER 

class vulkanEngine{
public:
    //vulkan dynamic loader
    vk::DynamicLoader vk_dynamic_loader;
    
    //vk::UniqueInstance instance;
    vk::UniqueInstance instance;
    vk::PhysicalDevice physical_device;
    vk::UniqueDevice device;
    
    /*queue*/
    uint32_t queue_family_index = 0;
    vk::Queue queue; //graphics + compute
                     
    /*command*/
    vk::UniqueCommandPool cmd_pool;
    std::vector<vk::UniqueCommandBuffer> cmd_bufs;
    

    /*layer + Extentin + feature*/
    std::vector<const char *> inst_required_layers;
    std::vector<const char *> inst_required_extentions;

    std::vector<const char *> dev_required_layers;
    std::vector<const char *> dev_required_extentions;
    vk::PhysicalDeviceFeatures2 physical_device_features2;
    
    /*glfw window*/
    glfwWindow* window;
    vk::UniqueSurfaceKHR surface;
    vk::SurfaceCapabilitiesKHR surface_capabilities;
    vk::SurfaceFormatKHR surface_format; 
    vk::PresentModeKHR surface_present_mode;
    
    /*vma allocator*/
    VmaAllocator vma_allocator;
    
    /*limit*/
    vk::PhysicalDeviceProperties physical_device_props;

public:
    vulkanEngine(){};
    virtual ~vulkanEngine(){vmaDestroyAllocator(vma_allocator);};
    
    void init(glfwWindow* w){
        window = w;

        /*init vulkan*/         
        addInstanceLayers();
        addInstanceExtentions();
        addDeviceLayers(); 
        addDeviceExtentions();
        addDeviceFeatures();

        /*init vulkan*/         
        createInstance();       if(w != NULL) createSurface();
        pickPhysicalDevice();   if(w != NULL) getSurfaceInfo();
        createLogicalDevice();
        physical_device_props = physical_device.getProperties();
        createQueue();
        createCommandPool();
        allocCommandBuffers();
        initVmaAllocator();
    }
    
    void createInstance(){
        auto app_info = vk::ApplicationInfo()
                        .setPApplicationName("my_application")
                        .setPEngineName("my_engine")
                        .setApiVersion(VK_API_VERSION_1_2);

        auto inst_create_info = vk::InstanceCreateInfo()
                                .setPApplicationInfo(&app_info);

        setInstanceLayers(inst_create_info);
        setInstanceExtentions(inst_create_info);

        #ifdef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
        PFN_vkGetInstanceProcAddr vk_get_instance_proc_addr 
            = vk_dynamic_loader.getProcAddress<PFN_vkGetInstanceProcAddr>( "vkGetInstanceProcAddr" );
        VULKAN_HPP_DEFAULT_DISPATCHER.init( vk_get_instance_proc_addr );

        instance = vk::createInstanceUnique(inst_create_info);
        VULKAN_HPP_DEFAULT_DISPATCHER.init( *instance );
        #endif
    }
    
    void pickPhysicalDevice(){
        std::vector<vk::PhysicalDevice> physical_devices = instance->enumeratePhysicalDevices();
             
        for(auto pd : physical_devices){
            if(isPhysicalDeviceSuitable(pd)){
                physical_device = pd;
                break;
            }
        }
    }
    
    void createLogicalDevice(){
        vk::DeviceQueueCreateInfo queue_create_info[1];
        float queue_priorities[1] = {1.0f};
        queue_create_info[0].setQueueFamilyIndex(queue_family_index)
                            .setQueueCount(1)
                            .setPQueuePriorities(queue_priorities);
        
        vk::DeviceCreateInfo dev_create_info;
        dev_create_info .setPQueueCreateInfos(queue_create_info)
                        .setQueueCreateInfoCount(1);

        setDeviceLayers(dev_create_info);
        setDeviceExtentions(dev_create_info);
        setDeviceFeatures(dev_create_info);
        
        device = physical_device.createDeviceUnique(dev_create_info);

        #ifdef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
        VULKAN_HPP_DEFAULT_DISPATCHER.init( *device );
        #endif
    }
    
    void createQueue(){
        queue = device->getQueue(queue_family_index, 0);
    }
    
    void createCommandPool(){
        cmd_pool = device->createCommandPoolUnique(
            vk::CommandPoolCreateInfo()
                .setQueueFamilyIndex(queue_family_index)
                .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        );
    }
    
    void allocCommandBuffers(){
        cmd_bufs = device->allocateCommandBuffersUnique(
            vk::CommandBufferAllocateInfo()
                .setCommandPool(cmd_pool.get())
                .setCommandBufferCount(8)
                .setLevel(vk::CommandBufferLevel::ePrimary)
        );
    } 
     
    void createSurface(){
        VkSurfaceKHR s = window->getVkSurface(&instance);
        surface = vk::UniqueSurfaceKHR{s,instance.get()};
        if(surface.get() == VK_NULL_HANDLE)exitStr("surface = null\n");
    } 
    
    void getSurfaceInfo(){
        std::vector<vk::SurfaceFormatKHR> surface_formats = physical_device.getSurfaceFormatsKHR(surface.get());
        std::vector<vk::PresentModeKHR> surface_present_modes = physical_device.getSurfacePresentModesKHR(surface.get());
        if(surface_formats.empty()||surface_present_modes.empty()){
            exitStr("get surface info error\n");
        }

        surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(surface.get());

        bool found_format = false;
        for(auto& format: surface_formats){
            if(format.format==vk::Format::eR8G8B8A8Unorm
            && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear){
                surface_format = format;
                found_format = true;
            }
        }
        
        bool found_pre_mode = false;
        for(auto& present_mode: surface_present_modes){
            if(present_mode == vk::PresentModeKHR::eFifo){
                surface_present_mode = present_mode;
                found_pre_mode = true;
            }
        }
        //printf("%s\n",vk::to_string(surface_formats[0].format).c_str());
        //printf("%s\n",vk::to_string(surface_formats[0].colorSpace).c_str());
        if(!found_format)exitStr("no suitable surface format\n");
        if(!found_pre_mode)exitStr("no suitable surface present mode\n");
    }
    
    void initVmaAllocator(){
        
        VmaVulkanFunctions vk_funtions = {};
        vk_funtions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
        vk_funtions.vkGetDeviceProcAddr   = &vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo allocator_create_info = {};
        allocator_create_info.instance          = instance.get();         
        allocator_create_info.physicalDevice    = physical_device;
        allocator_create_info.device            = device.get();
        allocator_create_info.vulkanApiVersion  = VK_API_VERSION_1_2; 
        allocator_create_info.pVulkanFunctions  = &vk_funtions;
        
        for(auto ext: dev_required_extentions){
            if(std::string_view(ext) == std::string_view(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)){
                allocator_create_info.flags 
                    = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            }
        }
        
        if(vmaCreateAllocator(&allocator_create_info, &vma_allocator)!=VK_SUCCESS)
            exitStr("cant init vma allocator\n");
    }
    
    void oneTimeSubmit(std::function<void(vk::UniqueCommandBuffer&)> inst){

        vk::CommandBufferAllocateInfo tmp_cmd_buf_alloc_info;
        tmp_cmd_buf_alloc_info.setCommandPool(cmd_pool.get())
                            .setCommandBufferCount(1)
                            .setLevel(vk::CommandBufferLevel::ePrimary);

        std::vector<vk::UniqueCommandBuffer> tmp_cmd_bufs 
            = device->allocateCommandBuffersUnique(tmp_cmd_buf_alloc_info);
        vk::CommandBufferBeginInfo cmdBeginInfo; 
        cmdBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        tmp_cmd_bufs[0]->begin(cmdBeginInfo); 
        inst(tmp_cmd_bufs[0]);
        tmp_cmd_bufs[0]->end();

        vk::CommandBuffer submit_cmd_buf[1] = {tmp_cmd_bufs[0].get()};
        vk::SubmitInfo submit_info;
        submit_info.setCommandBufferCount(1)
                .setPCommandBuffers(submit_cmd_buf);

        queue.submit({submit_info});
        queue.waitIdle();
    }
    

    void cmdChangeImageLayout(vk::CommandBuffer& cmd_buf,
                        vk::Image image, 
                        vk::ImageLayout old_layout,
                        vk::ImageLayout new_layout){
        
        vk::ImageMemoryBarrier img_memory_barrier{};
        img_memory_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                        .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                        .setImage(image)
                        .setOldLayout(old_layout)
                        .setNewLayout(new_layout)
                        .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor,0,1,0,1});
        
        // source layouts (old)
        switch (old_layout){
            case vk::ImageLayout::eUndefined:
                img_memory_barrier.srcAccessMask = {};
                break;
             case vk::ImageLayout::ePreinitialized:
                img_memory_barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
                break;
            case vk::ImageLayout::eColorAttachmentOptimal:
                img_memory_barrier.srcAccessMask =
                    vk::AccessFlagBits::eColorAttachmentWrite;
                break;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                img_memory_barrier.srcAccessMask =
                    vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                break;
            case vk::ImageLayout::eTransferSrcOptimal:
                img_memory_barrier.srcAccessMask =
                    vk::AccessFlagBits::eTransferRead;
                break;
            case vk::ImageLayout::eTransferDstOptimal:
                img_memory_barrier.srcAccessMask =
                    vk::AccessFlagBits::eTransferWrite;
                break;
            case vk::ImageLayout::eShaderReadOnlyOptimal:
                img_memory_barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
                break; 
            case vk::ImageLayout::ePresentSrcKHR:
                break;
            case vk::ImageLayout::eGeneral:
                break;
            default:
                exitStr("change image layout error: undefied old layout\n");
                break;
        }
        
        // Target layouts (new)
        switch (new_layout) {
            case vk::ImageLayout::eTransferDstOptimal:
                img_memory_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                break;
            case vk::ImageLayout::eTransferSrcOptimal:
                img_memory_barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
                break;
            case vk::ImageLayout::eColorAttachmentOptimal:
                img_memory_barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
                break;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                img_memory_barrier.dstAccessMask =
                    img_memory_barrier.dstAccessMask | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                break;
            case vk::ImageLayout::eShaderReadOnlyOptimal:
                if (img_memory_barrier.srcAccessMask == vk::AccessFlags{}) {
                    img_memory_barrier.srcAccessMask =
                        vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
                }
                img_memory_barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                break;
            case vk::ImageLayout::ePresentSrcKHR:
                break;
            case vk::ImageLayout::eGeneral:
                break;
            default:
                exitStr("change image layout error: undefied new layout\n");
                break;
        }
        
        vk::PipelineStageFlags src_stage_mask = vk::PipelineStageFlagBits::eAllCommands;
        vk::PipelineStageFlags dst_stage_mask = vk::PipelineStageFlagBits::eAllCommands;
        cmd_buf.pipelineBarrier(src_stage_mask,dst_stage_mask,{},{},{},img_memory_barrier);
    }

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

        return device->createShaderModuleUnique(shader_create_info);
    }

    


    
protected:
    /***utility***/
    void printAllDeviceName(std::vector<vk::PhysicalDevice> physical_devices){
        for (auto d: physical_devices){ 
            printf("%s\n",d.getProperties().deviceName.data());
        }
    }

    void printDeviceQueue(vk::PhysicalDevice pd){
        std::vector<vk::QueueFamilyProperties> queue_props = pd.getQueueFamilyProperties();

        printf("queue family count: %d\n\n",(int)queue_props.size());

        for(int i=0; i<queue_props.size(); i++){
            std::cout << "\tqueue family index: "   << i << std::endl;
            std::cout << "\t\tqueue count: "      << queue_props[i].queueCount << std::endl;
            std::cout << "\t\tgraphic support: "  << (queue_props[i].queueFlags & vk::QueueFlagBits::eGraphics ? "True" : "False") << std::endl;
            std::cout << "\t\tcompute support: "  << (queue_props[i].queueFlags & vk::QueueFlagBits::eCompute ? "True" : "False") << std::endl;
            std::cout << "\t\ttransfer support: " << (queue_props[i].queueFlags & vk::QueueFlagBits::eTransfer ? "True" : "False") << std::endl;
            std::cout << std::endl;
        }
    }
    
    bool isPhysicalDeviceSuitable(vk::PhysicalDevice pd){
        /*check queue*/
        if(!checkQueue(pd))exitStr("no suitable queue\n");
        
        /*check extentions*/
        if(!checkDeviceExtentions(pd))exitStr("not support extention\n");
        
        /*check features*/
        if(!checkDeviceFeatures(pd))exitStr("not support feature\n");
        
        /*result*/
        return true;
    }
    
    bool checkQueue(vk::PhysicalDevice pd){
        std::vector<vk::QueueFamilyProperties> queueProps = pd.getQueueFamilyProperties();

        for(int i=0; i<queueProps.size(); i++){
            vk::QueueFamilyProperties qp = queueProps[i];
            
            bool surface_support=true;
            if(window!=NULL)surface_support = pd.getSurfaceSupportKHR(i,surface.get());

            if( qp.queueFlags & vk::QueueFlagBits::eGraphics
                && qp.queueFlags & vk::QueueFlagBits::eCompute
                && surface_support){
                
                queue_family_index = i;//record queue index
                return true;
            }
        }
        return false;
    }
    
    bool checkDeviceExtentions(vk::PhysicalDevice pd){
        std::vector<vk::ExtensionProperties> ext_props = pd.enumerateDeviceExtensionProperties();

        for(auto req_ext : dev_required_extentions){
            for(size_t i=0; i<ext_props.size(); i++){
                if(std::string_view(req_ext) == std::string_view(ext_props[i].extensionName.data())){
                    break;
                }else if(i == ext_props.size()-1){
                    return false;
                    printf("no extention\n");
                }
            }
        }       
        return true;
    }
    
    bool checkDeviceFeatures(vk::PhysicalDevice pd){
        //あとでやります
        //auto f = pd.getFeatures2();
        //printf("%s\n",vk::to_string(f.sType).c_str());
        return true;
    }

    void setInstanceLayers(vk::InstanceCreateInfo& inst_create_info){
        #ifdef VK_DEBUG_LAYER
        inst_required_layers.push_back("VK_LAYER_KHRONOS_validation");
        #endif

        inst_create_info.setEnabledLayerCount((uint32_t)inst_required_layers.size())
                        .setPpEnabledLayerNames(inst_required_layers.data());
    }
    void setInstanceExtentions(vk::InstanceCreateInfo& inst_create_info){
        inst_create_info.setEnabledExtensionCount((uint32_t)inst_required_extentions.size())
                        .setPpEnabledExtensionNames(inst_required_extentions.data());
    }
    
    void setDeviceLayers(vk::DeviceCreateInfo& dev_create_info){
        #ifdef VK_DEBUG_LAYER
        dev_required_layers.push_back("VK_LAYER_KHRONOS_validation");
        #endif

        dev_create_info .setEnabledLayerCount((uint32_t)dev_required_layers.size())
                        .setPpEnabledLayerNames(dev_required_layers.data());
    } 
    void setDeviceExtentions(vk::DeviceCreateInfo& dev_create_info){
        dev_create_info.setEnabledExtensionCount((uint32_t)dev_required_extentions.size())
                        .setPpEnabledExtensionNames(dev_required_extentions.data());
    }
    void setDeviceFeatures(vk::DeviceCreateInfo& dev_create_info){
        dev_create_info.setPEnabledFeatures(NULL);//enable all feature in vk1.0
        dev_create_info.setPNext(&physical_device_features2);
    }
     
    virtual void addInstanceLayers() = 0;
    virtual void addInstanceExtentions() = 0;

    virtual void addDeviceLayers() = 0; 
    virtual void addDeviceExtentions() = 0; 
    virtual void addDeviceFeatures() = 0;  
};

