#include "Include/entity.h"

#include <vector>

#include "unordered_finder.h"
#include "logger.h"

static std::unordered_map<std::string, std::shared_ptr<Component>> components;
static std::vector<std::string> componentNames;

void Entity::AddComponent(std::unique_ptr<Component> p_component, const std::string& name)
{
    components[name];

    componentNames.push_back(name);

    auto& currentComponent = *FindUnorderedMap(name, components);
    currentComponent = std::move(p_component);
    currentComponent->Start();
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
