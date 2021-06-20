#include "Include/material_component.h"
#include "material_manager.h"


void MaterialComponent::CreateMaterial(const std::string& name)
{
    MaterialManager::CreateMaterial(name);
}

std::vector<std::string>& MaterialComponent::GetMaterials()
{
    return MaterialManager::GetMaterials();
}

Material& MaterialComponent::GetMaterial(const std::string& name)
{
    return MaterialManager::GetMaterial(name);
}

void MaterialComponent::Start()
{

}

void MaterialComponent::Update(const float& deltaTime)
{

}

void MaterialComponent::Destroy()
{
}