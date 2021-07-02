#pragma once 

#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "texture.h"

struct TextureCheck
{
    int textures[6];
};

class Material 
{
public:
    TextureCheck textureCheck;
    void SetColor(const glm::vec4& input)
    {
        color = input;
        updated = true;
    };
    glm::vec4 GetColor() {return color;}
    int GetRepeateCount() {return repeateCount;}
    std::vector<std::string> GetTextures() {return textures;}

    void SetTextures(const std::vector<std::string>& inputs, const std::vector<ColorFormat>& formats)
    {
        textures = inputs;
        textureFormats = formats;
        updated = true;
    };

    std::vector<ColorFormat>& GetFormats() {return textureFormats;}

    void SetRepeateCount(const int& input)
    {
        repeateCount = input;
        updated = true;
    }

    void SetEmission(const glm::vec4& input){emission = input; updated = true;}
    glm::vec4 GetEmission() {return emission;}

    void SetEmissionPower(const float& input){emissionPower = input; updated = true;}
    float GetEmissionPower() {return emissionPower;}

    void SetAo(const float& input){ao = input; updated = true;}
    float GetAo() {return ao;}

    void SetRoughness(const float& input){roughness = input; updated = true;}
    float GetRoughness() {return roughness;}

    void SetMetallic(const float& input){metallic = input; updated = true;}
    float GetMetallic() {return metallic;}

    void ResetUpdate() {updated = false;}
    void UpdateMaterial() {updated = true;}
    bool isUpdated() { return updated;}

    bool& GetTextureUpdate() {return textureUpdate;}
    void UpdateTextures(const bool& input) {textureUpdate = input;}

    void ExecuteTextures();
    
    std::vector<Texture> textureObjects;
    uint32_t offset = 0;
private:
    glm::vec4 color = glm::vec4(1.0f);
    glm::vec4 emission = glm::vec4(1.0f);

    float emissionPower = 0.0f;
    float roughness = 0.5f;
    float metallic = 0.5f;
    float ao = 1.0f;
    
    std::vector<std::string> textures;
    std::vector<ColorFormat> textureFormats;
    int repeateCount = 1;

    bool updated = false;

    bool textureUpdate = false;
};

struct MaterialManager
{
    static void Init();
    static void CreateMaterial(const std::string& materialName);
    static Material& GetMaterial(const std::string& materialName);
    static std::vector<std::string>& GetMaterials();
    static void AddTextures(const std::string& materialName, const std::string& samplerName = "default sampler");
    static void BindMaterial(const std::string& materialName);
    static void DeleteMaterial(const std::string& materialName);
    static void DeleteAllMaterials();
};



