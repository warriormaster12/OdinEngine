#pragma once

#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include "renderer.h"


struct GPUCameraData
{
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
	glm::mat4 modelMatrix;
};


class Camera {
public:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 velocity;
	glm::vec3 inputAxis;

	glm::vec3 forward;
	glm::vec3 up;

	glm::vec4 viewPos;

	float pitch{ 0 }; //up-down rotation
	float yaw{ 0 }; //left-right rotation

    float Fov = 70;
	float zNear = 0.1f;
	float zFar = 100.0f;

	bool bSprint = false;
	bool possessCamera = false;


	void ProcessInputEvent();
	void UpdateCamera(float deltaTime);


	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix(bool bReverse = false, bool flipY =true) const;
	glm::mat4 GetRotationMatrix() const;

	AllocatedBuffer cameraBuffer;
};
