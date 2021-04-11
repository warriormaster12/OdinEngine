#pragma once 

#include "vk_meshhandler.h"


struct GPUObjectData
{
    glm::mat4 modelMatrix;
};

struct Mesh
{
    AllocatedBuffer indexBuffer;
    AllocatedBuffer vertexBuffer;

    AllocatedBuffer meshBuffer;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    void CreateMesh();

    void DestroyMesh();

    void LoadFromObj(const std::string& path);
};