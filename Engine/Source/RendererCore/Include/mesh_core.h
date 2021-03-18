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

    glm::vec3 position;

	glm::mat4 transformMatrix;

    bool shadowEnabled = true;
};

namespace RendererCore
{
    void CreateMeshFromFile(const std::string& name, const std::string& filePath, VulkanRenderer& vkRenderer);
    void CreateMesh(const std::string& name, Mesh& inputMesh, VulkanRenderer& vkRenderer);
    Mesh* GetMesh(const std::string& name);

    std::vector<RenderObject> GetRenderObjects();
    void CreateRenderObject(RenderObject& inputObject);
}