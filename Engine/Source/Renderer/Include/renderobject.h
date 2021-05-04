#pragma once 

#include "mesh.h"

#include <iostream>
#include <string>

struct RenderObject
{
    Mesh* mesh = nullptr;
    std::string material;
};

struct ObjectManager 
{
    static void PushObjectToQueue(RenderObject& object);

    static void RenderObjects();
};