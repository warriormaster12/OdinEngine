#pragma once 
#include <iostream>
#include <string>

#include <vector>

#include "entity.h"

struct Scene
{
    void AddEntity(const std::string& name);
    Entity* GetEntity(const std::string& name);
    std::vector<std::string>& GetEntities();
    void UpdateEntities(const float& deltaTime);
    void RenameEntity(const std::string& oldName, const std::string& newName);
    void DestroyEntities();
    void DestroyEntity(const std::string& name);
};
