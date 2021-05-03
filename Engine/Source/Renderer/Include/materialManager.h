#pragma once 

#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "texture.h"


class Material 
{
public:
    glm::vec4 color;
    std::vector<std::string> textures = {"", ""};
    
    std::vector<Texture> textureObjects;
};

struct MaterialManager
{
    static void CreateMaterial(const std::string& materialName);
    static Material& GetMaterial(const std::string& materialName);
    static void UpdateTextures(const std::string& materialName);
    static void BindMaterial(const std::string& materialName);
    static void DeleteMaterial(const std::string& materialName);
};



