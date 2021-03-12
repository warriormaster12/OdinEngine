#pragma once 

#include "vk_mesh.h"
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


class VulkanRenderer;
struct Material;

struct RenderObject {
	Mesh* p_mesh;

	Material* p_material;

	glm::mat4 transformMatrix;

    bool shadowEnabled = true;
};

namespace RendererCore
{
    void CreateMeshFromFile(const std::string& name, const std::string& filePath, VulkanRenderer& vkRenderer);
    void CreateMesh(const std::string& name, Mesh& inputMesh, VulkanRenderer& vkRenderer);
    Mesh* GetMesh(const std::string& name);

    std::vector<RenderObject> GetRenderObjects();
    RenderObject CreateRenderObject(const std::string& meshName, const std::string& materialName, glm::mat4 m_transformMatrix);
}