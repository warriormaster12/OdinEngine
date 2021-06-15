#pragma once 
#include <iostream>
#include <string>

#include "entity.h"

struct Scene
{
    void AddEntity(const std::string& name);
    Entity* GetEntity(const std::string& name);
    void UpdateEntities(const float& deltaTime);
    void DestroyEntities();
};
