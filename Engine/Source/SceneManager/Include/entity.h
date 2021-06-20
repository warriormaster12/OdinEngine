#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


#include "component.h"


class Entity
{
public:
    void AddComponent(std::unique_ptr<Component> p_component, const std::string& name);

    Component* GetComponent(const std::string& name);

    std::vector<std::string>& GetComponents();
    
    void Update(const float& deltaTime);

    void Destroy();
private: 
    std::unordered_map<std::string, std::shared_ptr<Component>> components;
    std::vector<std::string> componentNames;
};
