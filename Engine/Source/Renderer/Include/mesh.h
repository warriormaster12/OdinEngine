#pragma once 

#include "vk_meshhandler.h"

struct Mesh
{
    AllocatedBuffer indexBuffer;
    AllocatedBuffer vertexBuffer;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    void CreateMesh();

    void DestroyMesh();
};