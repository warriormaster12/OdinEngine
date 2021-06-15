#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "component.h"


struct Entity 
{
    void AddComponent(std::unique_ptr<Component> p_component, const std::string& name);
    
    void Update(const float& deltaTime);

    void Destroy();
};
