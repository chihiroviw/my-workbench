#pragma once
#include "vulkanEngine.hpp"
#include "memoryVma.hpp"
#include <map>
#include <deque>

//my desctiptor rules
//set 0 = uniform buffer (vec3,...)
//set 1 = TLAS
//set 2 = output image
//set 3 = ...


//desc_set_layouts -> desc_pool -> desc_sets -> update 
struct uniqueDescriptorSets{

    vulkanEngine* ve;
    std::vector<std::vector<vk::DescriptorSetLayoutBinding>> desc_sets_bindings;

    //desc_set_layouts -> desc_pool -> desc_sets -> update 
    std::vector<vk::UniqueDescriptorSetLayout> uni_desc_set_layouts;
    std::vector<vk::DescriptorSetLayout> desc_set_layouts;
    
    //descriptor pool
    std::map<vk::DescriptorType,uint32_t> desc_type_count;
    vk::UniqueDescriptorPool desc_pool;

    //desc_sets
    std::vector<vk::UniqueDescriptorSet> uni_desc_sets;
    std::vector<vk::DescriptorSet> desc_sets;

    /*writer*/
    std::vector<std::vector<vk::WriteDescriptorSet>> writers;

    /*update writer info*/
    //これはupdateのたびにリセットされる
    std::vector<vk::WriteDescriptorSet> update_writers;
    std::deque<vk::WriteDescriptorSetAccelerationStructureKHR> accel_infos;
    std::deque<vk::DescriptorBufferInfo> buffer_infos;
    std::deque<vk::DescriptorImageInfo> image_infos;


public:
    uniqueDescriptorSets(){}

    uniqueDescriptorSets(vulkanEngine* ve, 
                   std::vector<std::vector<vk::DescriptorSetLayoutBinding>>&& arg_desc_sets_bindings)
                   :ve(ve),
                   desc_sets_bindings(std::move(arg_desc_sets_bindings)){
        
        createDescsetLayouts();
        createDescriptorPool();
        createDescriptorSets();
        createWriters();
    }
    
    void createDescsetLayouts(){
        for(auto& desc_set_bindings: desc_sets_bindings){
            vk::DescriptorSetLayoutCreateInfo info;
            info.setBindings(desc_set_bindings);
            
            uni_desc_set_layouts.push_back(ve->device->createDescriptorSetLayoutUnique(info));
            desc_set_layouts.push_back(*uni_desc_set_layouts.back());
        }
    }
    
    void createDescriptorPool(){
        //count descriptor type
        for(auto& desc_set_bindings: desc_sets_bindings){
            for(auto& desc_set_binding: desc_set_bindings){

                auto type = desc_set_binding.descriptorType;

                if(desc_type_count.contains(type)){
                    desc_type_count[type] += 1;
                }else{
                    desc_type_count.insert(std::make_pair(type,1));                   
                }
            }
        }
        
        //create desc pool
        std::vector<vk::DescriptorPoolSize> desc_pool_sizes; 
        for(auto [type, count]: desc_type_count){
            desc_pool_sizes.push_back({type,count});
        }
        
        vk::DescriptorPoolCreateInfo desc_pool_create_info;
        desc_pool_create_info.setPoolSizes(desc_pool_sizes)
                            .setMaxSets((uint32_t)desc_sets_bindings.size())
                            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
        desc_pool = ve->device->createDescriptorPoolUnique(desc_pool_create_info);
    }
    
    void createDescriptorSets(){
        vk::DescriptorSetAllocateInfo allocate_info;
        
        allocate_info.setDescriptorPool(*desc_pool)
                    .setSetLayouts(desc_set_layouts);

        uni_desc_sets = ve->device->allocateDescriptorSetsUnique(allocate_info);
        
        for(auto& uni_desc_set: uni_desc_sets){
            desc_sets.push_back(uni_desc_set.get());
        }
    }
    
    void createWriters(){

        auto max_sets = desc_sets_bindings.size();
        writers.resize(max_sets);

        for(auto set=0; set<max_sets; set++){

            auto max_binds = desc_sets_bindings[set].size();
            writers[set].resize(max_binds);

            for(auto binding=0; binding<max_binds; binding++){
                auto type = desc_sets_bindings[set][binding].descriptorType;
                writers[set][binding] = vk::WriteDescriptorSet()
                                            .setDstSet(desc_sets[set])
                                            .setDstBinding(binding)
                                            .setDescriptorCount(1)
                                            .setDescriptorType(type);
            }
        }
    }
    
    //update accel info
    uniqueDescriptorSets& addUpdateInfo(uint32_t set, 
                                        uint32_t binding, 
                                        uniqueAccelStruct& top_as
                                        ){
        auto writer = writers.at(set).at(binding);
        
        //check type
        if(writer.descriptorType != vk::DescriptorType::eAccelerationStructureKHR){
            exitStr("addUpdateInfo AS: worng type\n");
        }
        
        vk::WriteDescriptorSetAccelerationStructureKHR accel_info;
        accel_info.setAccelerationStructures(*top_as.accel_struct);

        accel_infos.push_back(accel_info);

        writer.setPNext(&accel_infos.back());
        update_writers.push_back(writer);

        return *this;
    }
    
    //update image info
    uniqueDescriptorSets& addUpdateInfo(uint32_t set, 
                                        uint32_t binding, 
                                        vk::ImageView image_view,
                                        vk::ImageLayout image_layout
                                        ){

        auto writer = writers.at(set).at(binding);
        
        //check type
        if( image_layout == vk::ImageLayout::eGeneral
            && writer.descriptorType != vk::DescriptorType::eStorageImage){
            exitStr("addUpdateInfo image: layout dont match desc type\n");
        }
        
        vk::DescriptorImageInfo image_info;
        image_info.setImageView(image_view)
                .setImageLayout(image_layout);
        
        image_infos.push_back(image_info);

        writer.setPImageInfo(&image_infos.back());
        update_writers.push_back(writer);
        
        return *this;
    }
    
    
    //update uniform info
    uniqueDescriptorSets& addUpdateInfo(uint32_t set, 
                                        uint32_t binding, 
                                        uniqueBufferVma& buffer){
 
        auto writer = writers.at(set).at(binding);
        
        //check type
        if(writer.descriptorType != vk::DescriptorType::eUniformBuffer
            && writer.descriptorType != vk::DescriptorType::eStorageBuffer){
            exitStr("addUpdateInfo Buffer: worng type\n");
        }
        
        if(buffer.buffer == VK_NULL_HANDLE){
            exitStr("addUpdateInfo Buffer: NULL HANDLE\n");
        }

        vk::DescriptorBufferInfo buffer_info;
        buffer_info.setBuffer(buffer.buffer)
                .setOffset(0)
                .setRange(buffer.buf_size);
        
        buffer_infos.push_back(buffer_info);

        writer.setPBufferInfo(&buffer_infos.back());
        update_writers.push_back(writer);

        return *this;       
    }
    
    void update(){
        ve->device->updateDescriptorSets(update_writers, nullptr);

        update_writers.resize(0);
        accel_infos.resize(0);
        buffer_infos.resize(0);
        image_infos.resize(0);
    }
};