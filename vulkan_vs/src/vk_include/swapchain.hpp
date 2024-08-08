#pragma once
#include "vulkanEngine.hpp"

struct uniqueSwapchain{
public:
    //vulkan engine
    vulkanEngine* ve;
    
    //swapchain
    vk::UniqueSwapchainKHR swapchain;

    uint32_t img_idx;
    std::vector<vk::Image> swapchain_images;
    std::vector<vk::UniqueImageView> swapchain_imgviews;
    std::vector<vk::UniqueFramebuffer> swapchain_framebufs;

public:    
    uniqueSwapchain(){}

    //use framebuffer (for common renderpass)
    uniqueSwapchain(vulkanEngine* ve, vk::UniqueRenderPass& render_pass):ve(ve){
        createSwapchain(vk::ImageUsageFlagBits::eColorAttachment); 
        setSwapchainImages();
        createSwapchainImgviews(vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity);
        createSwapchainFramebuffers(render_pass);
    }
    
    //for raytracing pipeline
    uniqueSwapchain(vulkanEngine* ve):ve(ve){
        createSwapchain(vk::ImageUsageFlagBits::eStorage); 
        setSwapchainImages();
        createSwapchainImgviews(vk::ComponentSwizzle::eR,
                                vk::ComponentSwizzle::eG,
                                vk::ComponentSwizzle::eB,
                                vk::ComponentSwizzle::eA);
        
        ve->oneTimeSubmit(
            [&](vk::UniqueCommandBuffer& cb){
                for(auto image: swapchain_images){
                    ve->cmdChangeImageLayout(*cb,
                                        image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::ePresentSrcKHR);
                }
            }
        );
    }
    
    
    
    void createSwapchain(vk::ImageUsageFlags usage){
        swapchain = ve->device->createSwapchainKHRUnique(
            vk::SwapchainCreateInfoKHR()
                .setSurface(ve->surface.get())
                .setMinImageCount(ve->surface_capabilities.minImageCount+1)
                .setImageFormat(ve->surface_format.format)
                .setImageColorSpace(ve->surface_format.colorSpace)
                .setImageExtent(ve->surface_capabilities.currentExtent)
                .setImageArrayLayers(1)
                .setImageUsage(usage)
                .setImageSharingMode(vk::SharingMode::eExclusive)
                .setPreTransform(ve->surface_capabilities.currentTransform)
                .setPresentMode(ve->surface_present_mode)
                .setClipped(VK_TRUE)
                .setQueueFamilyIndices(ve->queue_family_index)
        );
    }

    void createSwapchainImgviews(vk::ComponentSwizzle r,
                                vk::ComponentSwizzle g,
                                vk::ComponentSwizzle b,
                                vk::ComponentSwizzle a){

        for(auto swapchain_img: swapchain_images){

            swapchain_imgviews.push_back(ve->device->createImageViewUnique(

                vk::ImageViewCreateInfo()
                    .setImage(swapchain_img)
                    .setViewType(vk::ImageViewType::e2D)
                    .setFormat(ve->surface_format.format)
                    .setComponents(vk::ComponentMapping(r, g, b, a))//r,g,b,a
                    .setSubresourceRange(vk::ImageSubresourceRange(
                                        vk::ImageAspectFlagBits::eColor,//ImageAspectFlags aspectMask 
                                        0,  //baseMipLevel
                                        1,  //levelCount
                                        0,  //baseArrayLaeyr
                                        1)) //layerCount
            ));
        }
    }
    
    void createSwapchainFramebuffers(vk::UniqueRenderPass& render_pass){
        for (auto &swapchain_imgview: swapchain_imgviews) {

            vk::ImageView frameBufAttachments[1] = {swapchain_imgview.get()};

            swapchain_framebufs.push_back(ve->device->createFramebufferUnique(
                vk::FramebufferCreateInfo()
                    .setWidth(ve->surface_capabilities.currentExtent.width)
                    .setHeight(ve->surface_capabilities.currentExtent.height)
                    .setLayers(1)
                    .setRenderPass(*render_pass)
                    .setAttachmentCount(1)
                    .setPAttachments(frameBufAttachments)
            ));
        }
    }
    
    void setSwapchainImages(){
        swapchain_images = ve->device->getSwapchainImagesKHR(*swapchain);
    }
    
    void acquireNextImage(vk::UniqueSemaphore& next_img_ready_semaphore){
        auto acquire_img_result = ve->device->acquireNextImageKHR(
                                    swapchain.get(), 1'000'000'000,//1second(ns)
                                    next_img_ready_semaphore.get());

        if (acquire_img_result.result != vk::Result::eSuccess)exitStr("cant get next frame.\n");
        img_idx = acquire_img_result.value;
    } 
    

    auto nextRawFramebuffer(){return swapchain_framebufs[img_idx].get();}
    auto nextRawImageView(){return swapchain_imgviews[img_idx].get();}
    auto nextImage(){return swapchain_images[img_idx];}
    
    void presentation(vk::UniqueSemaphore& wait_semaphore){
        //presentation
        ve->queue.presentKHR(
            vk::PresentInfoKHR()
                .setSwapchainCount(1)
                .setPSwapchains(&swapchain.get()) 
                .setPImageIndices(&img_idx)
                .setWaitSemaphoreCount(1)
                .setPWaitSemaphores(&wait_semaphore.get())
        );
    }
    
    
public:
    // move assignment operator  
    uniqueSwapchain& operator=(uniqueSwapchain&& other){
        ve = other.ve;

        swapchain           = std::move(other.swapchain);
        
        img_idx             = other.img_idx;
        swapchain_images    = std::move(other.swapchain_images);
        swapchain_imgviews  = std::move(other.swapchain_imgviews);
        swapchain_framebufs = std::move(other.swapchain_framebufs);
        
        return *this;
    }
};