#pragma once

#include "component.h"

#include <vector>

#include <string>

#include "material_manager.h"


class MaterialComponent : public Component
{
public:
    virtual void Start() override;

    virtual void Update(const float& deltaTime) override;

    virtual void Destroy() override;

    void CreateMaterial(const std::string& name);

    std::string& GetMaterialName() {return currentMaterial;}

    void SetMaterialName(const std::string& input) {currentMaterial = input;}

    Material& GetMaterial(const std::string& name);

    std::vector<std::string>& GetMaterials();
private: 
    std::string currentMaterial = "default material";
};