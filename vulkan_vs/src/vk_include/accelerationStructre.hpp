#pragma once
#include <vulkan/vulkan.hpp>
#include "memoryVma.hpp"
#include "vertexIndexBuffer.hpp"

//prot declear...
VkTransformMatrixKHR convertTransform(glm::mat3x4& m);

//-----------------------
//  struct(+methods)
//-----------------------

struct uniqueAccelStruct{
    vk::UniqueAccelerationStructureKHR accel_struct;
    uniqueBufferVma buffer_as;
    vk::DeviceAddress address;
    
    uniqueAccelStruct(){address = 0;};
    
    uniqueAccelStruct(vulkanEngine* ve,
                vk::AccelerationStructureTypeKHR type,
                std::vector<vk::AccelerationStructureGeometryKHR>& geometrys,
                std::vector<uint32_t>& primitive_counts){ 
                //BLAS->num of triangle par geometry, 
                //TLAS->num of BLAS(instance) par geometry
                
        //get build info
        vk::AccelerationStructureBuildGeometryInfoKHR geometry_build_info;
        geometry_build_info.setType(type)
                        .setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
                        .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
                        .setGeometries(geometrys);
        
        auto build_sizes = ve->device->getAccelerationStructureBuildSizesKHR(
                                        vk::AccelerationStructureBuildTypeKHR::eDevice, 
                                        geometry_build_info,
                                        primitive_counts);
        
        //create buffer for AS
        buffer_as = uniqueBufferVma(ve,
                            build_sizes.accelerationStructureSize,
                            vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
                            |vk::BufferUsageFlagBits::eShaderDeviceAddress,
                            VMA_MEMORY_USAGE_GPU_ONLY);

        //create AS
        vk::AccelerationStructureCreateInfoKHR create_info;
        create_info .setBuffer(buffer_as.buffer) 
                    .setSize(build_sizes.accelerationStructureSize)
                    .setType(type);
        accel_struct = ve->device->createAccelerationStructureKHRUnique(create_info);

        //create scratch buffer
        /*  Due to a validation layer bug, 
            an error related to the offset alignment of the scratch buffer device address may occur, 
            but it should not be a problem. */
        auto features2 = ve->physical_device.getProperties2<vk::PhysicalDeviceProperties2,
                                        vk::PhysicalDeviceAccelerationStructurePropertiesKHR>();
        auto as_prop = features2.get<vk::PhysicalDeviceAccelerationStructurePropertiesKHR>();
        uniqueBufferVma scratch_buffer(ve,
                                build_sizes.buildScratchSize,
                                vk::BufferUsageFlagBits::eStorageBuffer
                                |vk::BufferUsageFlagBits::eShaderDeviceAddress,
                                VMA_MEMORY_USAGE_GPU_ONLY,
                                as_prop.minAccelerationStructureScratchOffsetAlignment);

        //build
        geometry_build_info.setDstAccelerationStructure(*accel_struct)
                        .setScratchData(scratch_buffer.address);
        
        std::vector<vk::AccelerationStructureBuildRangeInfoKHR> build_range_infos; 
                               
        for(auto pc: primitive_counts){
            vk::AccelerationStructureBuildRangeInfoKHR build_range_info;
            build_range_info.setPrimitiveCount(pc) 
                            .setPrimitiveOffset(0)
                            .setFirstVertex(0)
                            .setTransformOffset(0);

            build_range_infos.push_back(build_range_info);
        }
        
        ve->oneTimeSubmit(
            [&](vk::UniqueCommandBuffer& cb){
                //one geometry build info
                cb->buildAccelerationStructuresKHR(geometry_build_info, build_range_infos.data());                
            }
        );
        
        //get address
        vk::AccelerationStructureDeviceAddressInfoKHR address_info;
        address_info.setAccelerationStructure(*accel_struct);
        address = ve->device->getAccelerationStructureAddressKHR(address_info);
    }
};


struct uniqueTopAccelStruct{
    std::vector<uniqueAccelStruct> bottom_ASs;
    uniqueAccelStruct top_as;
};


//-----------------------
//  utility
//-----------------------

uniqueAccelStruct createBottomLevelAS(vulkanEngine* ve,
                                std::vector<uniquePolygonMesh>& polygon_meshs,
                                std::vector<uint32_t>& tree_mesh){
    
    //geometorys info
    std::vector<vk::AccelerationStructureGeometryKHR> geometrys;
    std::vector<uint32_t> primitive_counts;
    
    for(auto mesh: tree_mesh){

        auto& vertex_buf    = polygon_meshs[mesh].vert_buf;
        auto& index_buf     = polygon_meshs[mesh].idx_buf;

        //create geometry
        vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
        triangles.setVertexStride(vertex_buf.stride)
                .setMaxVertex(vertex_buf.maxVertex())
                .setVertexFormat(vertex_buf.format())
                .setVertexData(vertex_buf.buffer.address)

                .setIndexType(index_buf.format())
                .setIndexData(index_buf.buffer.address);
                //.setTransformData(nullptr);//not recommend 
                                           
        //one BLAS can have some geometrys
        vk::AccelerationStructureGeometryKHR geometry;
        geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles)
                .setGeometry(triangles)
                .setFlags(vk::GeometryFlagBitsKHR::eOpaque);
        
        uint32_t primitive_count = index_buf.elem_count/3;
        
        geometrys.push_back(geometry);
        primitive_counts.push_back(primitive_count);
    }
    
    //create + build BLAS
    return uniqueAccelStruct(ve,
                    vk::AccelerationStructureTypeKHR::eBottomLevel,
                    geometrys,
                    primitive_counts);
}

uniqueAccelStruct createTopLevelAS(vulkanEngine* ve, 
                            std::vector<uniqueAccelStruct>& bottom_ASs,
                            meshTree& mesh_tree){



    //instances info
    std::vector<vk::AccelerationStructureInstanceKHR> accel_instances;
    
    for(uint32_t i=0; i<bottom_ASs.size(); i++){

        vk::TransformMatrixKHR transform = convertTransform(mesh_tree.tree[i].transform); 

        //create instance
        vk::AccelerationStructureInstanceKHR accel_instance;
        accel_instance.setTransform(transform)
                    .setInstanceCustomIndex(mesh_tree.tree[i].mesh_idx)
                    .setMask(0xFF)
                    .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable)
                    .setAccelerationStructureReference(bottom_ASs[i].address)
                    //ここでメッシュの参照hit_groupを変える
                    .setInstanceShaderBindingTableRecordOffset(mesh_tree.tree[i].hit_group_idx);

        accel_instances.push_back(accel_instance);
    }

    uniqueBufferVma instances_buffer(ve,
                        sizeof(vk::AccelerationStructureInstanceKHR)*bottom_ASs.size(),
                        accel_instances.data(),
                        vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR|
                        vk::BufferUsageFlagBits::eShaderDeviceAddress,
                        VMA_MEMORY_USAGE_GPU_ONLY,
                        16);
    

    vk::AccelerationStructureGeometryInstancesDataKHR instances_data;
    instances_data.setArrayOfPointers(VK_FALSE)   
                .setData(instances_buffer.address);

    //If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, geometryCount must be 1 
    std::vector<vk::AccelerationStructureGeometryKHR> geometrys(1);//must be 1
    geometrys[0].setGeometryType(vk::GeometryTypeKHR::eInstances)
                .setGeometry(instances_data)
                .setFlags(vk::GeometryFlagBitsKHR::eOpaque);

    std::vector<uint32_t> primitive_counts(1);
    primitive_counts[0] = bottom_ASs.size();

    //create + build TLAS
    return uniqueAccelStruct(ve,
                    vk::AccelerationStructureTypeKHR::eTopLevel,
                    geometrys,
                    primitive_counts);
}

VkTransformMatrixKHR convertTransform(glm::mat3x4& m){
    VkTransformMatrixKHR mtx{};
    memcpy(&mtx.matrix[0], &m[0], sizeof(float) * 4);
    memcpy(&mtx.matrix[1], &m[1], sizeof(float) * 4);
    memcpy(&mtx.matrix[2], &m[2], sizeof(float) * 4);
    
    return mtx;
}

uniqueTopAccelStruct createTlasBlas(vulkanEngine* ve, meshTree& mesh_tree){

    uniqueTopAccelStruct tlas;

    for(auto& blas_tree: mesh_tree.tree){
        tlas.bottom_ASs.push_back(createBottomLevelAS(ve,
                                                mesh_tree.polygon_meshs,
                                                blas_tree.blas_meshs));
    }

    tlas.top_as = createTopLevelAS(ve, tlas.bottom_ASs, mesh_tree);   
    return std::move(tlas);
}
