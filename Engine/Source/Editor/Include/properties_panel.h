#pragma once 

#include <string>

#include "entity.h"

struct PropertiesPanel
{
    static void ShowPropertiesPanelWindow(Entity& entity, const std::string& entityName);
};