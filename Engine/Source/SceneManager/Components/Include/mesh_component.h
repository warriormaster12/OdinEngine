#pragma once 

#include "component.h"

#include <vector>

#include "render_object.h"


class MeshComponent : public Component
{
public:
    virtual void Start() override;

    virtual void Update(const float& deltaTime) override;

    virtual void Destroy() override;

    void AddMesh(const std::string& path, const std::string& materialName ="");

    void ReAddMesh(RenderObject& object);

    std::vector<RenderObject>& GetMeshes();

};