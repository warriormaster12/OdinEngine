#include "Include/scene.h"
#include "Include/entity.h"

#include <vector>
#include <unordered_map>

#include "unordered_finder.h"
#include "logger.h"

static std::unordered_map<std::string, Entity> entities;
static std::vector<std::string> entityNames;


void Scene::AddEntity(const std::string& name)
{
    entities[name];
    entityNames.push_back(name);
}

Entity* Scene::GetEntity(const std::string& name)
{
    if(FindUnorderedMap(name,entities) != nullptr)
    {
        return FindUnorderedMap(name,entities); 
    }
    else {
        ENGINE_CORE_WARN("couldn't find entity by the name {0}", name);
        return nullptr;
    }
    
}

void Scene::UpdateEntities(const float& deltaTime)
{
    for(auto& currentName : entityNames)
    {
        auto& currentEntity = *FindUnorderedMap(currentName, entities);
        currentEntity.Update(deltaTime);
    }
}