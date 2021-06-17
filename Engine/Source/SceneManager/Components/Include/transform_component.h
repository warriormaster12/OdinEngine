#pragma once 

#include "component.h"

#include <vector>

#include <glm/glm.hpp>


class  Transform3D : public Component
{
public:
    virtual void Start() override;

    virtual void Update(const float& deltaTime) override;

    virtual void Destroy() override;

    void UpdateTranslation(const glm::vec3& input) {
        translation = input;
    }

    glm::vec3& GetTranslation() {return translation;}

private:
    glm::vec3 translation = glm::vec3(0);
};