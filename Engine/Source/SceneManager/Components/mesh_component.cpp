#include "Include/mesh_component.h"

#include "glm/gtx/transform.hpp"


#include "logger.h"
#include "render_object.h"
#include <memory>
#include <filesystem>


void MeshComponent::Start()
{

}

void MeshComponent::AddMesh(const std::string& path, const std::string& materialName /*=""*/)
{
    std::filesystem::path pathObj(path);
    RenderObject object = {};
    object.p_mesh->LoadFromObj(path);
    object.p_mesh->CreateMesh();
    object.meshName = pathObj.filename().string();
    object.material = "Test";
    object.transformMatrix = glm::translate(glm::vec3(0));
    ObjectManager::PushObjectToQueue(object);
}

void MeshComponent::ReAddMesh(RenderObject& object)
{
    ObjectManager::PushObjectToQueue(object);
}

std::vector<RenderObject>& MeshComponent::GetMeshes()
{
    return ObjectManager::GetObjects();
}

void MeshComponent::Update(const float& deltaTime)
{

}

void MeshComponent::Destroy()
{
}
