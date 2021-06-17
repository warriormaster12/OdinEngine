#pragma once 

#include "component.h"

#include <vector>

#include "render_object.h"

#include "transform_component.h"


class MeshComponent : public Component
{
public:
    virtual void Start() override;

    virtual void Update(const float& deltaTime) override;

    virtual void Destroy() override;

    void AddMesh(const std::string& path, const std::string& entity, const std::string& materialName ="", Transform3D* p_transfrom = nullptr);

    void UpdateCurrentMesh(const std::string& entity, const std::string& materialName ="", Transform3D* p_transfrom = nullptr);

};