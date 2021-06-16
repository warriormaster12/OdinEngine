#pragma once 

#include <iostream>
#include <string>

#include "scene.h"

struct SceneManager
{
    static void CreateScene(const std::string& name);
    static Scene& GetScene(const std::string& name);
};