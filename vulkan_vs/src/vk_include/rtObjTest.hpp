#pragma once

#include "vulkanEngine.hpp"
#include "vertexIndexBuffer.hpp"


//-----------------------
//  struct
//-----------------------

struct vertexPNX{
    glm::f32vec3 position;
    glm::f32vec3 normal;
    glm::f32vec3 v3_coord;
};
//data
#include "../utility/obj/cubeData.hpp"

//-----------------------
//  utility
//-----------------------
meshTree prepareScene(vulkanEngine* ve){

    std::vector<vertexPNX> vertex0 =   {
        {{-500, -500, -500}   ,{0,0,0}    ,{1,0,0}},
        {{500, -500, -500}   ,{0,0,0}    ,{0,1,0}},
        {{0.0 , 0.75*500, -500}  ,{0,0,0}    ,{0,0,1}},
    };

    std::vector<vertexPNX> vertex1 =   {
        {{1.0f, 1.0f, 0.0f}     ,{0,0,0}    ,{0,0,1}},
        {{-1.0f, 1.0f, 0.0f}    ,{0,0,0}    ,{0,0,1}},
        {{0.0f, -1.0f, -1.0f}    ,{0,0,0}    ,{1,0,0}},
    };
    
    std::vector<uint32_t> index = {0,1,2};

    auto flags = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
                |vk::BufferUsageFlagBits::eStorageBuffer
                |vk::BufferUsageFlagBits::eShaderDeviceAddress;
    
    meshTree mesh_tree;

    mesh_tree.polygon_meshs.push_back(uniquePolygonMesh(ve, 
                            vertex0.data(), sizeof(vertexPNX), vertex0.size(), 
                            index.data(), index.size(),
                            flags));
                                
    mesh_tree.polygon_meshs.push_back(uniquePolygonMesh(ve, 
                            vertex1.data(), sizeof(vertexPNX), vertex1.size(), 
                            index.data(), index.size(),
                            flags));
    
    mesh_tree.updateObjectParams(ve);
    
    mesh_tree.tree = {
        {
            {0},//blas{mesh idxs}   
            glm::f32mat3x4{
                {1.0f,0.0f,0.0f,0.0f},
                {0.0f,1.0f,0.0f,0.0f},
                {0.0f,0.0f,1.0f,0.0f}},
            0,  //hit_group
            0   //mesh_idx
        },
        {
            {1},
            glm::f32mat3x4{
                {1.0f,0.0f,0.0f,0.5f},
                {0.0f,1.0f,0.0f,0.0f},
                {0.0f,0.0f,2.0f,1.0f}},
            0,
            1
        },
    };
    
    return std::move(mesh_tree);
}


//make sphere
void makeSphere(   std::vector<vertexPNX>& vert, 
                    std::vector<uint32_t>& idx,
                    int slices, int stacks, 
                    glm::vec3 color){

	uint32_t vertices = (slices+1)*(stacks+1);
	uint32_t faces = slices*stacks*2;

	//GLfloat Position[3];
	//Object::Vertex;
	
	vert.resize(vertices);
	idx.resize(sizeof(uint32_t)*3*faces);

	/**頂点の位置**/
    auto cur = 0;
	for (int j = 0; j<=stacks; j++){
		float ph = 3.14159265*(float)j/(float)stacks;
		float y = cosf(ph);
		float r = sinf(ph);

		for (int i = 0; i<=slices; i++){
			float th = 2.0f * 3.14159265*(float)i/(float)slices;
			float x = r*cosf(th);
			float z = r*sinf(th);

            vertexPNX tmp = {{x,y,z},{x,y,z},color};
            vert[cur] = tmp;
            cur++;
		}
	}

	/**面の指標**/
    cur = 0;
	for (int j = 0; j<stacks; j++){
		for (int i = 0; i<slices; i++){
			int count = (slices+1) * j + i;

			idx[cur+0] = count;
			idx[cur+1] = count+1;
			idx[cur+2] = count+slices+2;
			cur += 3;

			idx[cur+0] = count;
			idx[cur+1] = count+slices+2;
			idx[cur+2] = count+slices+1;
			cur += 3;
		}
	}
}

void makePlane( std::vector<vertexPNX>& vert,
                std::vector<uint32_t>& idx,
                glm::f32vec3 origin,
                glm::f32vec3 u,
                glm::f32vec3 v,
                glm::f32vec3 color){
    
    
    auto normal = glm::normalize(cross(u,v));

    auto p0 = origin;
    auto p1 = origin + u;
    auto p2 = origin + v;
    auto p3 = origin + u + v;
    
    vertexPNX vpnx0 = {p0,normal,color};
    vertexPNX vpnx1 = {p1,normal,color};
    vertexPNX vpnx2 = {p2,normal,color};
    vertexPNX vpnx3 = {p3,normal,color};
    
    auto offset = vert.size();
    
    vert.push_back(vpnx0);
    vert.push_back(vpnx1);
    vert.push_back(vpnx2);
    vert.push_back(vpnx3);
    
    idx.push_back(offset+0);
    idx.push_back(offset+1);
    idx.push_back(offset+2);
    idx.push_back(offset+1);
    idx.push_back(offset+2);
    idx.push_back(offset+3);
}

void makeCube( std::vector<vertexPNX>& vert,
                std::vector<uint32_t>& idx){
    for(auto& v: solid_cube_vertex){
        vert.push_back(v);
    }
    
    for(auto& i: solid_cube_index){
        idx.push_back(i);
    }
}

//cornell box
meshTree prepareSceneCornell(vulkanEngine* ve){
    auto red   = glm::vec3(.65, .05, .05);
    auto white = glm::vec3(.73, .73, .73);
    auto green = glm::vec3(.12, .45, .15);
    auto light = glm::vec3(1, 1, 1);

    
    std::vector<vertexPNX> vertex;
    std::vector<uint32_t> index;
    
    makePlane(vertex,index,{555,0,0}, {0,555,0}, {0,0,555}, green);
    makePlane(vertex,index,{0,0,0}, {0,555,0}, {0,0,555}, red);
    makePlane(vertex,index,{343, 554, 332}, {-130,0,0}, {0,0,-105}, light);
    makePlane(vertex,index,{0,0,0}, {555,0,0}, {0,0,555}, white);
    makePlane(vertex,index,{555,555,555}, {-555,0,0}, {0,0,-555}, white);
    makePlane(vertex,index,{0,0,555}, {555,0,0}, {0,555,0}, white);

    std::vector<vertexPNX> vertex1;
    std::vector<uint32_t> index1;
    
    makeSphere(vertex1,index1,10,10,white);
    
    
    auto flags = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
                |vk::BufferUsageFlagBits::eStorageBuffer
                |vk::BufferUsageFlagBits::eShaderDeviceAddress;
    
    meshTree mesh_tree;

    mesh_tree.polygon_meshs.push_back(uniquePolygonMesh(ve, 
                            vertex.data(), sizeof(vertexPNX), vertex.size(), 
                            index.data(), index.size(),
                            flags));
    

    mesh_tree.polygon_meshs.push_back(uniquePolygonMesh(ve, 
                            vertex1.data(), sizeof(vertexPNX), vertex1.size(), 
                            index1.data(), index1.size(),
                            flags));

    mesh_tree.updateObjectParams(ve);
    
    mesh_tree.tree = {
        {
            {0},//blas{mesh idxs}   
            glm::f32mat3x4{
                {1.0f,0.0f,0.0f,0.0f},
                {0.0f,1.0f,0.0f,0.0f},
                {0.0f,0.0f,1.0f,0.0f}},
            0,  //hit_group
            0   //mesh_idx
        }, 
        {
            {1},//blas{mesh idxs}   
            glm::f32mat3x4{
                {100.0f,0.0f,0.0f,0},
                {0.0f,100.0f,0.0f,0},
                {0.0f,0.0f,100.0f,0}},
            0,  //hit_group
            1   //mesh_idx
        }, 
    };
    
    return std::move(mesh_tree);
}

//new scene
meshTree prepareSceneTest0(vulkanEngine* ve){
    auto red   = glm::f32vec3(.65, .05, .05);
    auto white = glm::f32vec3(.73, .73, .73);
    auto green = glm::f32vec3(.12, .45, .15);
    auto light = glm::f32vec3(1, 1, 1);


    auto flags = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
                |vk::BufferUsageFlagBits::eStorageBuffer
                |vk::BufferUsageFlagBits::eShaderDeviceAddress;
    
    //make vertex index
    std::vector<vertexPNX> vertices0;
    std::vector<uint32_t> indices0;
    makeCube(vertices0,indices0);
    auto polygon_mesh0  =  uniquePolygonMesh(ve, 
                                            vertices0.data(), sizeof(vertexPNX), vertices0.size(), 
                                            indices0.data(), indices0.size(),
                                            flags);
    polygon_mesh0.setParameterVector({red,0});//ambient

    //make vertex index
    std::vector<vertexPNX> vertices1;
    std::vector<uint32_t> indices1;
    makeCube(vertices1,indices1);
    makePlane(vertices1,indices1,{5+8,-8,5+8}, {-20,0,0}, {0,0,-20}, green);
    auto polygon_mesh1  =  uniquePolygonMesh(ve, 
                                            vertices1.data(), sizeof(vertexPNX), vertices1.size(), 
                                            indices1.data(), indices1.size(),
                                            flags);
    polygon_mesh1.setParameterVector({green,0});//ambient
    

    //make vertex index
    std::vector<vertexPNX> vertices2;
    std::vector<uint32_t> indices2;
    makeSphere(vertices2,indices2,40,40,white);
    auto polygon_mesh2  =  uniquePolygonMesh(ve, 
                                            vertices2.data(), sizeof(vertexPNX), vertices2.size(), 
                                            indices2.data(), indices2.size(),
                                            flags);
    polygon_mesh2.setParameterVector({white,0});//ambient
                                                

    //make polygon mesh
    meshTree mesh_tree;
    mesh_tree.polygon_meshs.push_back(std::move(polygon_mesh0));
    mesh_tree.polygon_meshs.push_back(std::move(polygon_mesh1));
    mesh_tree.polygon_meshs.push_back(std::move(polygon_mesh2));
    

    mesh_tree.updateObjectParams(ve);
    
    mesh_tree.tree = {
        {
            {0},//blas{mesh idxs}   
            glm::f32mat3x4{
                {200.0f,0.0f,0.0f,300.0f},
                {0.0f,200.0f,0.0f,0.0f},
                {0.0f,0.0f,200.0f,0.0f}},
            0,  //hit_group
            0   //mesh_idx
        }, 
        {
            {1},//blas{mesh idxs}   
            glm::f32mat3x4{
                {200.0f,0.0f,0.0f,-600.0f},
                {0.0f,200.0f,0.0f,0.0f},
                {0.0f,0.0f,200.0f,-100.0f}},
            0,  //hit_group
            1   //mesh_idx
        }, 
        {
            {2},//blas{mesh idxs}   
            glm::f32mat3x4{
                {200.0f,0.0f,0.0f,0.0f},
                {0.0f,200.0f,0.0f,-800.0f},
                {0.0f,0.0f,200.0f,-600.0f}},
            0,  //hit_group
            2   //mesh_idx
        }, 
        {
            {2},//blas{mesh idxs}   
            glm::f32mat3x4{
                {400.0f,0.0f,0.0f,0.0f},
                {0.0f,400.0f,0.0f,0.0f},
                {0.0f,0.0f,400.0f,-900.0f}},
            1,  //hit_group
            2   //mesh_idx
        }, 
    };
    
    return std::move(mesh_tree);
}

/*notes*/
/*
//	I	= Ia + Id + Is
//	Ia	= Ka x La
//	Id	= max(N*L, 0) x Kd x Ld
//	Is	= max(N*H, 0)^Ns x Ks x Ls
//	H	= (L+V).normalize
//	L	= frag_position->Lpos
//	V	= frag_position->camera(0,0,0)

//shading
uniform vec4 Lpos;
uniform vec3 La;
uniform vec3 Ld;
uniform vec3 Ls;

layout (std140) uniform Material{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Ns;
};



in vec4 view_model_position;
in vec3 view_model_normal;
in vec2 uv;

void main() {
	
	vec3 N = view_model_normal;
	vec4 P = view_model_position;

	//Ia
	vec3 Ia = Ka*La;

	//Id
	vec3 L = normalize((Lpos*P.w - P*Lpos.w).xyz);
	vec3 Id = max(dot(N,L), 0)*Kd*Ld;

	//Is
	vec3 V = -normalize(P.xyz);
	vec3 H = normalize(V+L);
	vec3 Is = pow(max(dot(N, H), 0), Ns)*Ks*Ls;
}
*/

