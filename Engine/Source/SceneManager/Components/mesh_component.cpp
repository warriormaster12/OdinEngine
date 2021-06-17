#include "Include/mesh_component.h"

#include "glm/gtx/transform.hpp"


#include "logger.h"
#include "render_object.h"
#include <memory>
#include <filesystem>


void MeshComponent::Start()
{

}

void MeshComponent::AddMesh(const std::string& path, const std::string& entity,const std::string& materialName /*=""*/,Transform3D* p_transfrom /*= nullptr*/)
{
    if(ObjectManager::GetRenderObject(entity) == nullptr)
    {
        std::filesystem::path pathObj(path);
        RenderObject object = {};
        object.p_mesh->LoadFromObj(path);
        object.p_mesh->CreateMesh();
        object.material = "Test";
        if(p_transfrom != nullptr)
        {
            object.transformMatrix = glm::translate(p_transfrom->GetTranslation());
        }
        else {
            object.transformMatrix = glm::translate(glm::vec3(0));
        }
        
        ObjectManager::PushObjectToQueue(object, entity);
    }
        
}

void MeshComponent::UpdateCurrentMesh(const std::string& entity, const std::string& materialName /*=""*/, Transform3D* p_transfrom /*= nullptr*/)
{
    RenderObject& object = *ObjectManager::GetRenderObject(entity);
    if(p_transfrom != nullptr)
    {
        object.transformMatrix = glm::translate(p_transfrom->GetTranslation());
    }
    else {
        object.transformMatrix = glm::translate(glm::vec3(0));
    }
}

void MeshComponent::Update(const float& deltaTime)
{

}

void MeshComponent::Destroy()
{
}
