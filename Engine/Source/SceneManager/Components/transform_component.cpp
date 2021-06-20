#include "Include/transform_component.h"
#include "logger.h"


void Transform3D::Start()
{

}

void Transform3D::Update(const float& deltaTime)
{

}

void Transform3D::Destroy()
{
}

void Transform3D::UpdateTransform()
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    transform = glm::mat4{
    {
        scale.x * (c1 * c3 + s1 * s2 * s3),
        scale.x * (c2 * s3),
        scale.x * (c1 * s2 * s3 - c3 * s1),
        0.0f,
    },
    {
        scale.y * (c3 * s1 * s2 - c1 * s3),
        scale.y * (c2 * c3),
        scale.y * (c1 * c3 * s2 + s1 * s3),
        0.0f,
    },
    {
        scale.z * (c2 * s1),
        scale.z * (-s2),
        scale.z * (c1 * c2),
        0.0f,
    },
    {translation.x, translation.y, translation.z, 1.0f}};
}