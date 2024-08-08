#pragma once
#include <cstring>

#include "vulkanEngine.hpp"
#include "utility/error/err.hpp"

//allocate device memory
vk::UniqueDeviceMemory allocateDeviceMemory(vulkanEngine* ve,
                                            vk::MemoryRequirements buf_mem_req,      //buffer required memory types
                                            vk::MemoryPropertyFlagBits req_mem_prop_flags){//required memory properties
 
        vk::PhysicalDeviceMemoryProperties mem_props = ve->physical_device.getMemoryProperties();
        
        //適切なヒープを探す,ヒープはメモリの置かれる領域の実体のこと
        //用途に合わせてdevice localなヒープを持つメモリのインデックスを探す（逆も然り
        uint32_t heap_index=0;
        for(uint32_t i = 0; i < mem_props.memoryTypeCount; i++){
            auto is_heap_device_local =(mem_props.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal); 

            if( req_mem_prop_flags&vk::MemoryPropertyFlagBits::eDeviceLocal
                &&is_heap_device_local){ //device local
                heap_index = i;
                break;
            }

            if( !(req_mem_prop_flags&vk::MemoryPropertyFlagBits::eDeviceLocal) 
                && !is_heap_device_local){//host visible
                heap_index = i;
                break;
            }
        }

        //良さげなメモリを探す
        bool suitable_memory_type_found = false;
        uint32_t memory_type_index = 0;
        for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((buf_mem_req.memoryTypeBits & (0x01 << i)) &&                   //使えるメモリタイプのうち
                (mem_props.memoryTypes[i].propertyFlags & req_mem_prop_flags)&& //prop flag bitの立っているもの
                (mem_props.memoryTypes[i].heapIndex == heap_index)) {           //ついでにメモリの置かれるヒープが要件を満たす

                memory_type_index = i;
                suitable_memory_type_found = true;
                break;
            }
        }

        if(!suitable_memory_type_found)exitStr("cant fount sutiable device memory");

        return std::move(ve->device->allocateMemoryUnique(
            vk::MemoryAllocateInfo()
                .setAllocationSize(buf_mem_req.size)
                .setMemoryTypeIndex(memory_type_index)
        ));
}




struct Buffer{
public:
    vk::UniqueBuffer buffer;
    vk::UniqueDeviceMemory buffer_memory; 
    size_t buf_size;
    void* mapped_memory;
    
    vulkanEngine* ve;
    bool host_visible;
    
    bool moved;

public:
    auto rawBuffer(){return buffer.get();}
    auto rawBufferMemory(){return buffer_memory.get();}

    ~Buffer(){unmapMemory();}

    Buffer(){}

    Buffer( vulkanEngine* ve, size_t size, 
            vk::BufferUsageFlagBits buffer_usage,vk::MemoryPropertyFlagBits req_mem_prop)
            :ve(ve),buf_size(size),mapped_memory(NULL),moved(false){
                
        if(req_mem_prop&vk::MemoryPropertyFlagBits::eHostVisible){
            host_visible = true;
            auto [buf, buf_mem] = createBuffer(size,buffer_usage,req_mem_prop);
            buffer = std::move(buf);
            buffer_memory = std::move(buf_mem);
            mapMemory();

        }else{
            host_visible = false;
            auto [buf, buf_mem] = createBuffer(size,buffer_usage|(vk::BufferUsageFlagBits::eTransferDst),req_mem_prop);
            buffer = std::move(buf);
            buffer_memory = std::move(buf_mem);
        }
    }

    Buffer( vulkanEngine* ve, size_t size, void* data,
            vk::BufferUsageFlagBits buffer_usage,vk::MemoryPropertyFlagBits mem_prop) 
            :Buffer(ve,size,buffer_usage,mem_prop){
        copyDataToMemory(data,size);
    }
    
    
    
    std::tuple<vk::UniqueBuffer,vk::UniqueDeviceMemory> 
    createBuffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlagBits req_mem_prop){

        vk::BufferCreateInfo buffer_create_info;
        buffer_create_info.setSize(size)
                        .setUsage(usage)
                        .setSharingMode(vk::SharingMode::eExclusive);

        auto buffer = ve->device->createBufferUnique(buffer_create_info);

        auto buf_mem_req = ve->device->getBufferMemoryRequirements(buffer.get()); 

        auto buf_memory = allocateDeviceMemory(ve,buf_mem_req,req_mem_prop);

        ve->device->bindBufferMemory(buffer.get(), buf_memory.get(), 0);
        
        return std::tuple(std::move(buffer),std::move(buf_memory));
    }; 
    
    //copy data
    void copyDataToMemory(void* data, size_t size){

        if(host_visible){//host visible memory -> use mapped memory
            std::memcpy(mapped_memory, data, size);
            ve->device->flushMappedMemoryRanges(vk::MappedMemoryRange{buffer_memory.get(),0,size});

        }else{//device local memory -> use staging buffer
            //create staging buffer + set data
            auto staging_buffer = Buffer(ve,size,data,
                                    vk::BufferUsageFlagBits::eTransferSrc,
                                    vk::MemoryPropertyFlagBits::eHostVisible);
            
            //copy staging buffer -> device local buffer
            vk::CommandPoolCreateInfo tmp_cmd_pool_create_info;
            tmp_cmd_pool_create_info.setQueueFamilyIndex(ve->queue_family_index)
                                    .setFlags(vk::CommandPoolCreateFlagBits::eTransient);
            vk::UniqueCommandPool tmp_cmd_pool = ve->device->createCommandPoolUnique(tmp_cmd_pool_create_info);

            vk::CommandBufferAllocateInfo tmp_cmd_buf_alloc_info;
            tmp_cmd_buf_alloc_info.commandPool = tmp_cmd_pool.get();
            tmp_cmd_buf_alloc_info.commandBufferCount = 1;
            tmp_cmd_buf_alloc_info.level = vk::CommandBufferLevel::ePrimary;

            std::vector<vk::UniqueCommandBuffer> tmp_cmd_bufs 
                = ve->device->allocateCommandBuffersUnique(tmp_cmd_buf_alloc_info);

            vk::BufferCopy buf_copy;
            buf_copy.setSrcOffset(0).setDstOffset(0).setSize(size);

            vk::CommandBufferBeginInfo cmdBeginInfo; 
            cmdBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

            tmp_cmd_bufs[0]->begin(cmdBeginInfo);
            tmp_cmd_bufs[0]->copyBuffer(staging_buffer.buffer.get(), buffer.get(), {buf_copy});
            tmp_cmd_bufs[0]->end();

            vk::CommandBuffer submit_cmd_buf[1] = {tmp_cmd_bufs[0].get()};
            vk::SubmitInfo submit_info;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = submit_cmd_buf;

            ve->queue.submit({submit_info});
            ve->queue.waitIdle();
        }
    }  
    
    //mapping host visible memory
    void mapMemory(){
        if(host_visible){
            mapped_memory = ve->device->mapMemory(buffer_memory.get(), 0, buf_size);
        }else{
            exitStr("cant map device local memory\n");
        }
    }

    //unmap memory
    void unmapMemory(){
        if(moved){//moved 
            return;
        }else if(host_visible==true && mapped_memory != NULL){
            ve->device->unmapMemory(buffer_memory.get());
        }else if(host_visible==true && mapped_memory == NULL){
            exitStr("cant unmap memory\n");
        }
    } 

    // move assignment operator  
    Buffer& operator=(Buffer && other){
        this->buffer = std::move(other.buffer);
        this->buffer_memory = std::move(other.buffer_memory);
        this->buf_size = other.buf_size;
        this->host_visible = other.host_visible; 
        this->mapped_memory = other.mapped_memory; 
        this->ve = other.ve;
        other.moved = true;
        return *this;
    } 
};
