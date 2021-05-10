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
    
    std::vector<std::string> textures;
    int repeateCount = 1;
    
    std::vector<Texture> textureObjects;
};

struct MaterialManager
{
    static void CreateMaterial(const std::string& materialName, const std::string& samplerName = "default sampler");
    static Material& GetMaterial(const std::string& materialName);
    static void UpdateTextures(const std::string& materialName, const std::string& samplerName = "default sampler");
    static void BindMaterial(const std::string& materialName);
    static void DeleteMaterial(const std::string& materialName);
};



