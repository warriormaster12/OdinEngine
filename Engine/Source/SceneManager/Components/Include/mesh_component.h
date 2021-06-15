#pragma once 

#include "component.h"


class MeshComponent : public Component
{
public:
    virtual void Start() override;

    virtual void Update(const float& deltaTime) override;

    virtual void Destroy() override;

    void AddMesh(const std::string& path);

};