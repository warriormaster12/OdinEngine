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
        ENGINE_CORE_ERROR("couldn't find entity by the name {0}", name);
        return nullptr;
    }
    
}

std::vector<std::string>& Scene::GetEntities()
{
    return entityNames;
}

void Scene::RenameEntity(const std::string& oldName, const std::string& newName)
{
    if(FindUnorderedMap(oldName, entities))
    {
        //copy application data from entity that we want to erase     
        auto& tempEntity = *FindUnorderedMap(oldName, entities);
        //create new entity and copy data
        entities[newName] = tempEntity;

        entities.erase(oldName);
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

void Scene::DestroyEntity(const std::string& name)
{
    if(FindUnorderedMap(name, entities) != nullptr)
    {
        auto& target = *FindUnorderedMap(name, entities);
        target.Destroy();
        entities.erase(name);
        for(int i = 0; i < entityNames.size(); i++)
        {
            if(entityNames[i] == name)
            {
                entityNames.erase(entityNames.begin() + i);
                entityNames.shrink_to_fit();
            }
        }
    }
}