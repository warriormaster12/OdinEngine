#include "Include/mesh_component.h"

#include "glm/gtx/transform.hpp"
#include "render_object.h"

#include "logger.h"
#include <memory>


void MeshComponent::Start()
{

}

void MeshComponent::AddMesh(const std::string& path)
{
    RenderObject object = {};
    object.p_mesh->LoadFromObj(path);
    object.p_mesh->CreateMesh();
    object.material = "Test";
    object.transformMatrix = glm::translate(glm::vec3(0));
    ObjectManager::PushObjectToQueue(object);
}

void MeshComponent::Update(const float& deltaTime)
{

}

void MeshComponent::Destroy()
{
}
