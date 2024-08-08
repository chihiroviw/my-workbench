//---------------------------
//  index
//---------------------------
layout(buffer_reference, buffer_reference_align = 4, scalar) readonly buffer Indices{
    uvec3 i[];
};

//---------------------------
//  vertex
//---------------------------
struct vertexPNX{
    vec3 position;
    vec3 normal;
    vec3 v3_coord;
};

layout(buffer_reference, buffer_reference_align = 4, scalar) readonly buffer verticesPNX{
    vertexPNX v[];
};

//---------------------------
//  fetch vertex utility
//---------------------------
vertexPNX fetchVertexInterleavedPNX(vec3 barys,
                                    uint64_t vertex_buffer_addr,
                                    uint64_t index_buffer_addr){

    Indices indices = Indices(index_buffer_addr);
    verticesPNX verts = verticesPNX(vertex_buffer_addr);
    
    uvec3 idx = indices.i[gl_PrimitiveID];
    vertexPNX v0 = verts.v[idx.x];
    vertexPNX v1 = verts.v[idx.y];
    vertexPNX v2 = verts.v[idx.z];
    
    vertexPNX cur_vertex;
    cur_vertex.position = v0.position*barys.x + v1.position*barys.y + v2.position*barys.z;
    cur_vertex.normal   = normalize(v0.normal*barys.x   + v1.normal*barys.y   + v2.normal*barys.z);
    cur_vertex.v3_coord = v0.v3_coord*barys.x + v1.v3_coord*barys.y + v2.v3_coord*barys.z;
    
    return cur_vertex;
}