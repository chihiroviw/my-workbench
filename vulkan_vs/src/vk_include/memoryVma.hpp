#pragma once
#include <cstring>

#include "vulkanEngine.hpp"
#include "utility/error/err.hpp"


struct uniqueBufferVma{
public:
    size_t buf_size;
    VkBuffer buffer;
    vk::DeviceAddress address;
    VmaAllocation buffer_allocation;

    vulkanEngine* ve;

    void* mapped_memory;

    bool host_visible;

public:  
    uniqueBufferVma():buffer(VK_NULL_HANDLE){}
    
    uniqueBufferVma(  vulkanEngine* ve, 
                size_t size,
                vk::BufferUsageFlags buffer_usage, 
                VmaMemoryUsage memory_usage,
                uint32_t aligment = 0)
                :ve(ve),buf_size(size),mapped_memory(NULL),buffer(VK_NULL_HANDLE){

        host_visible =  isHostVisible(memory_usage);

        if(host_visible){
            createBuffer(size,buffer_usage,memory_usage,aligment);
            mapMemoroy();
        }else{
            createBuffer(size,buffer_usage|vk::BufferUsageFlagBits::eTransferDst,memory_usage,aligment);
        }

        if(buffer_usage & vk::BufferUsageFlagBits::eShaderDeviceAddress){
            vk::BufferDeviceAddressInfoKHR addressInfo{};
            addressInfo.setBuffer(buffer);
            address = ve->device->getBufferAddress(&addressInfo);
        }
    }
    
    uniqueBufferVma(  vulkanEngine* ve, 
                size_t size, 
                void* data,
                vk::BufferUsageFlags buffer_usage, 
                VmaMemoryUsage memory_usage,
                uint32_t aligment = 0)
                :uniqueBufferVma(ve,size,buffer_usage,memory_usage, aligment){
        copyDataToMemory(data,size);
    }
    
    void createBuffer(  size_t size,
                        vk::BufferUsageFlags buffer_usage, 
                        VmaMemoryUsage memory_usage,
                        uint32_t aligment){
        
        VmaAllocationCreateInfo buffer_alloc_info = {};
        buffer_alloc_info.usage = memory_usage;
        
        if(host_visible){
            buffer_alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }
        
        VkBufferCreateInfo buffer_create_info = vk::BufferCreateInfo()
                                                .setSize(size)
                                                .setUsage(buffer_usage)
                                                .setSharingMode(vk::SharingMode::eExclusive);
        
        if(vmaCreateBufferWithAlignment( ve->vma_allocator,
                            &buffer_create_info,
                            &buffer_alloc_info,
                            aligment,
                            &buffer,
                            &buffer_allocation,
                            nullptr
                            )!=VK_SUCCESS) exitStr("cant create buffer\n");
    }
    
    void copyDataToMemory(void* data, size_t size){
        
        if(host_visible){//host visible memory -> use mapped memory
            std::memcpy(mapped_memory, data, size);

        }else{  //device local memory -> use staging buffer
                //
            //create staging buffer + set data
            auto staging_buffer = uniqueBufferVma(ve,size,data,
                                    vk::BufferUsageFlagBits::eTransferSrc,
                                    VMA_MEMORY_USAGE_CPU_TO_GPU);
                        
            //copy staging buffer -> device local buffer
            vk::BufferCopy buf_copy;
            buf_copy.setSrcOffset(0).setDstOffset(0).setSize(size);

            ve->oneTimeSubmit(
                [&](vk::UniqueCommandBuffer& cb){
                    cb->copyBuffer(staging_buffer.buffer, buffer, {buf_copy});
                }
            );

        }
    }
    
    void mapMemoroy(){
        if(vmaMapMemory(ve->vma_allocator,buffer_allocation,&mapped_memory)!=VK_SUCCESS)
            exitStr("cnat map memory\n");
    }
    
    void unmapMemory(){vmaUnmapMemory(ve->vma_allocator,buffer_allocation);}
    
    bool isHostVisible(VmaMemoryUsage memory_usage){
        if( memory_usage == VMA_MEMORY_USAGE_CPU_ONLY
            || memory_usage == VMA_MEMORY_USAGE_CPU_TO_GPU
            || memory_usage == VMA_MEMORY_USAGE_GPU_TO_CPU){
            return true;
        }                
        
        return false;
    }
    
    //assignment
    uniqueBufferVma& operator=(uniqueBufferVma& rhs) = delete;

    //move assignment
    uniqueBufferVma& operator=(uniqueBufferVma&& rhs) noexcept{
        this->buf_size          = rhs.buf_size;
        this->buffer            = rhs.buffer;
        this->address           = rhs.address;
        this->buffer_allocation = rhs.buffer_allocation;
        this->ve                = rhs.ve;
        this->mapped_memory     = rhs.mapped_memory;
        this->host_visible      = rhs.host_visible;
        rhs.buffer              = VK_NULL_HANDLE;
        return *this;
    }
    //move constructor
    uniqueBufferVma(uniqueBufferVma&& arg) noexcept{
        *this = std::move(arg);
    }

    ~uniqueBufferVma(){
        if(buffer==VK_NULL_HANDLE)return;
        if(host_visible)unmapMemory();
        vmaDestroyBuffer(ve->vma_allocator,buffer,buffer_allocation);
    }
};

