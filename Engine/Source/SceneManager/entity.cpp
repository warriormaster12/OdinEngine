#include "Include/entity.h"

#include "unordered_finder.h"
#include "mesh_component.h"
#include "logger.h"



void Entity::AddComponent(std::unique_ptr<Component> p_component, const std::string& name)
{
    if(FindUnorderedMap(name, components) == nullptr)
    {
        components[name];

        componentNames.push_back(name);

        auto& currentComponent = *FindUnorderedMap(name, components);
        currentComponent = std::move(p_component);
        currentComponent->Start();
    }
}

Component& Entity::GetComponent(const std::string& name)
{
    auto& currentComponent = *FindUnorderedMap(name, components);
    return *currentComponent;
}

std::vector<std::string>&  Entity::GetComponents()
{
    return componentNames;
}

void Entity::Update(const float& deltaTime)
{
    for(auto& currentName : componentNames)
    {
        auto& currentComponent = *FindUnorderedMap(currentName, components);
        currentComponent->Update(deltaTime);
    }
}

void Entity::Destroy()
{
    for(auto& currentName : componentNames)
    {
        auto& currentComponent = *FindUnorderedMap(currentName, components);
        currentComponent->Destroy();
    }
}
