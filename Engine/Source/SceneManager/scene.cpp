#include "Include/scene.h"
#include "Include/entity.h"

#include <vector>
#include <unordered_map>
#include <vector>

#include "unordered_finder.h"
#include "logger.h"

static std::unordered_map<std::string, std::shared_ptr<Entity>> entities;
static std::vector<std::string> entityNames;


void Scene::AddEntity(std::unique_ptr<Entity> p_entity,const std::string& name)
{
    entities[name];

    entityNames.push_back(name);

    auto& currentEntity = *FindUnorderedMap(name, entities);
    currentEntity = std::move(p_entity);
}

void Scene::UpdateEntities(const float& deltaTime)
{
    for(auto& currentName : entityNames)
    {
        auto& currentEntity = *FindUnorderedMap(currentName, entities);
        currentEntity->Update(deltaTime);
    }
}