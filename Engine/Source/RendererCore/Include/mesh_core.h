#pragma once 

#include "vk_mesh.h"
#include <iostream>

#include "material_core.h"


class VulkanRenderer;

struct RenderObject {
	Mesh* p_mesh;

	Material* p_material;

	glm::mat4 transformMatrix;
};

namespace RendererCore
{
    void CreateMeshFromFile(const std::string& name, const std::string& filePath, VulkanRenderer& vkRenderer);
    void CreateMesh(const std::string& name, Mesh& inputMesh, VulkanRenderer& vkRenderer);
    Mesh* GetMesh(const std::string& name);
}