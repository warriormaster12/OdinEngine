#include "Include/camera.h"

#include <glm/gtx/transform.hpp>

Camera::Camera(vkcomponent::SwapChain& swapChain)
{
    p_swapChain = &swapChain;
}
void Camera::ProcessInputEvent(WindowHandler& windowHandler)
{
	inputAxis = glm::vec3(0.0f);
	if(windowHandler.GetKInput(GLFW_KEY_W) == GLFW_PRESS)
	{
		inputAxis.x += 1.f;
	}
	if(windowHandler.GetKInput(GLFW_KEY_S) == GLFW_PRESS)
	{
		inputAxis.x -= 1.f;
	}
	
	if(windowHandler.GetKInput(GLFW_KEY_D) == GLFW_PRESS)
	{
		inputAxis.y += 1.f;
	}
	if(windowHandler.GetKInput(GLFW_KEY_A) == GLFW_PRESS)
	{
		inputAxis.y -= 1.f;
	}
	
	if(windowHandler.GetKInput(GLFW_KEY_Q) == GLFW_PRESS)
	{
		inputAxis.z -= 1.f;
	}
	if(windowHandler.GetKInput(GLFW_KEY_E) == GLFW_PRESS)
	{
		inputAxis.z += 1.f;
	}
	
	if(windowHandler.GetKInput(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		bSprint = true;
	}
	else
	{
		bSprint = false;
	}
	if (possessCamera)
	{
		if(windowHandler.mouseMotion == true)
		{
			pitch += windowHandler.yoffset * 0.003f;
			yaw += windowHandler.xoffset * 0.003f;
			// 1.56 radians is about equal to 89.x degrees
			if(pitch > 1.56f)
			{
				pitch = 1.56f;
			}
			if(pitch < -1.56f)
			{
				pitch = -1.56f;
			}
			windowHandler.mouseMotion = false;
		}
	}
	if(windowHandler.GetMInput(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		possessCamera = true;
		glfwSetInputMode(windowHandler.p_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else
	{
		possessCamera = false;
		glfwSetInputMode(windowHandler.p_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	

	inputAxis = glm::clamp(inputAxis, { -1.0,-1.0,-1.0 }, { 1.0,1.0,1.0 });
}

void Camera::UpdateCamera(float deltaTime)
{
	const float cam_vel = 0.001f + bSprint * 0.01;
	glm::vec3 forward = { 0,0,-cam_vel };
	glm::vec3 right = { cam_vel,0,0 };
	glm::vec3 up = { 0,cam_vel,0 };

	glm::mat4 cam_rot = get_rotation_matrix();

	forward = cam_rot * glm::vec4(forward, 0.f);
	right = cam_rot * glm::vec4(right, 0.f);
	

	velocity = inputAxis.x * forward + inputAxis.y * right + inputAxis.z * up;

	velocity *= 10 * deltaTime;

	position += velocity;
	

}


glm::mat4 Camera::GetViewMatrix() const
{
	glm::mat4 cam_rot = get_rotation_matrix();

	glm::mat4 view = glm::translate(glm::mat4{ 1 }, position) * cam_rot;

	//we need to invert the camera matrix
	view = glm::inverse(view);

	return view;
}

glm::mat4 Camera::GetOffscreenViewMatrix()
{
	glm::mat4 rotM = glm::mat4(1.0f);
	glm::mat4 transM;

	rotM = glm::rotate(rotM, glm::radians(rotation.x * (1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::vec3 translation = position;
	transM = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 view = rotM * transM;

	return view;
}

glm::mat4 Camera::GetProjectionMatrix(bool bReverse /*= true*/, bool flipY /*=true*/) const
{
	if (bReverse)
	{
		glm::mat4 pro = glm::perspective(glm::radians(Fov), (float)p_swapChain->actualExtent.width / (float)p_swapChain->actualExtent.height, zFar, zNear);
		if(flipY == true)
		{
			pro[1][1] *= -1;
		}
		return pro;
	}
	else {
		glm::mat4 pro = glm::perspective(glm::radians(Fov), (float)p_swapChain->actualExtent.width / (float)p_swapChain->actualExtent.height, zNear, zFar);
		if(flipY == true)
		{
			pro[1][1] *= -1;
		}
		return pro;
	}
}

glm::mat4 Camera::get_rotation_matrix() const
{
	glm::mat4 yaw_rot = glm::rotate(glm::mat4{ 1 }, yaw, { 0,-1,0 });
	glm::mat4 pitch_rot = glm::rotate(glm::mat4{ yaw_rot }, pitch, { -1,0,0 });

	return pitch_rot;
} 
