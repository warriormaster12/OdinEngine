#pragma once 

#include <string>

#include "entity.h"
#include "mesh_component.h"
#include "material_component.h"
#include "transform_component.h"

struct PropertiesPanel
{
    static void ShowPropertiesPanelWindow(Entity& entity, const std::string& entityName);
};