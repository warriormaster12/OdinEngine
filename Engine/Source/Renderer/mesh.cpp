#include "Include/mesh.h"
#include "Include/renderer.h"


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