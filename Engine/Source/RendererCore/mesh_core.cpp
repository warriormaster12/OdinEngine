#include "mesh_core.h"
#include "vk_renderer.h"
#include "material_core.h"

namespace RendererCore
{
    std::unordered_map<std::string, Mesh> meshes;
    std::vector<RenderObject> renderables;
    void CreateMeshFromFile(const std::string& name, const std::string& filePath, VulkanRenderer& vkRenderer)
    {
        Mesh outputMesh{};
        
        outputMesh.LoadFromObj(filePath.c_str());

        vkRenderer.UploadMesh(outputMesh);

        meshes[name] = outputMesh;
    }

    void CreateMesh(const std::string& name, Mesh& inputMesh, VulkanRenderer& vkRenderer)
    {
        vkRenderer.UploadMesh(inputMesh);
        meshes[name] = inputMesh;
    }

    Mesh* GetMesh(const std::string& name)
    {
        auto it = meshes.find(name);
        if (it == meshes.end()) {
            return nullptr;
        }
        else {
            return &(*it).second;
        }
    }

    std::vector<RenderObject> GetRenderObjects()
    {
        return renderables;
    }

    RenderObject CreateRenderObject(const std::string& meshName, const std::string& materialName, glm::mat4 m_transformMatrix)
    {
        RenderObject outpuObject;
        outpuObject.p_mesh = GetMesh(meshName);
        outpuObject.p_material = GetMaterial(materialName);
        outpuObject.transformMatrix = m_transformMatrix;

        renderables.push_back(outpuObject);
        
        return outpuObject;
    }
}