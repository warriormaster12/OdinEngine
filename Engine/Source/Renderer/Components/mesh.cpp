#include "Include/mesh.h"
#include "vk_meshhandler.h"
#include "renderer.h"

#include <tiny_obj_loader.h>




void Mesh::CreateMesh()
{
    if(Renderer::GetActiveAPI() == AvailableBackends::Vulkan)
    {
        VkMesh::UploadMeshData(vertices, vertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        VkMesh::UploadMeshData(indices, indexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }
}

void Mesh::DestroyMesh()
{
    if(Renderer::GetActiveAPI() == AvailableBackends::Vulkan)
    {
        VkMesh::DestroyBuffer(vertexBuffer);
        VkMesh::DestroyBuffer(indexBuffer);
    }
}

void Mesh::LoadFromObj(const std::string& path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error(warn + err);
    }
    
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            
            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            
            
            vertex.normal = 
            {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };
            
            
            vertex.uv = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            
            
            vertex.color = {1.0f, 1.0f, 1.0f};
            

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            

            
            indices.push_back(uniqueVertices[vertex]);

        }
    }
}