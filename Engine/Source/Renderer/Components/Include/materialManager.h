#pragma once 

#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "texture.h"


class Material 
{
public:
    
    void SetColor(const glm::vec4& input)
    {
        color = input;
        updated = true;
    };
    glm::vec4 GetColor() {return color;}
    int GetRepeateCount() {return repeateCount;}
    std::vector<std::string> GetTextures() {return textures;}

    void SetTextures(const std::vector<std::string>& inputs)
    {
        textures = inputs;
        updated = true;
    };

    void SetRepeateCount(const int& input)
    {
        repeateCount = input;
        updated = true;
    }

    void ResetUpdate() {updated = false;}
    bool isUpdated() { return updated;}
    
    std::vector<Texture> textureObjects;
    size_t materialByteOffset = 0;
private:
    glm::vec4 color = glm::vec4(1.0f);
    
    std::vector<std::string> textures;
    int repeateCount = 1;

    bool updated = false;
};

struct MaterialManager
{
    static void Init();
    static void CreateMaterial(const std::string& materialName, const std::string& samplerName = "default sampler");
    static Material& GetMaterial(const std::string& materialName);
    static void AddTextures(const std::string& materialName, const std::string& samplerName = "default sampler");
    static void BindMaterial(const std::string& materialName);
    static void DeleteMaterial(const std::string& materialName);
    static void DeleteAllMaterials();
};



