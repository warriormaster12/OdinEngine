#pragma once 
#include <iostream>
#include <memory>
#include <string>

#include "entity.h"

struct Scene
{
    void AddEntity(std::unique_ptr<Entity> p_entity, const std::string& name);
    void GetEntity();
    void UpdateEntities(const float& deltaTime);
    void DestroyEntities();
};
