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

    void UpdateRotation(const glm::vec3& input)
    {
        rotation = input;
    }

    void UpdateScale(const glm::vec3& input)
    {
        scale = input;
    }

    void UpdateTransform(Transform3D& input) {*this = input;}

    glm::mat4& GetTransform() {return transform;}

    void UpdateTransform();

    glm::vec3& GetTranslation() {return translation;}
    glm::vec3& GetRotation() {return rotation;}
    glm::vec3& GetScale() {return scale;}
private:
    glm::vec3 translation = glm::vec3(0);
    glm::vec3 rotation = glm::vec3(0);
    glm::vec3 scale = glm::vec3(1);

    glm::mat4 transform;
};