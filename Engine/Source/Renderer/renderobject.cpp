#include "Include/renderobject.h"
#include "Include/renderer.h"
#include "Include/materialManager.h"

std::vector<RenderObject> objects;

void ObjectManager::PushObjectToQueue(RenderObject& object)
{
    objects.push_back(object);
}

void ObjectManager::RenderObjects()
{
    for(auto& currentObject : objects)
    {
        MaterialManager::GetMaterial(currentObject.material).color = glm::vec4(1.0f);
        MaterialManager::BindMaterial(currentObject.material);
        Renderer::BindVertexBuffer(currentObject.mesh->vertexBuffer);
        Renderer::BindIndexBuffer(currentObject.mesh->indexBuffer);
        Renderer::DrawIndexed(currentObject.mesh->indices);
    }
}

