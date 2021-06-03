#include "Include/camera.h"



#include "logger.h"
#include "vk_swapchain.h"
#include "window_handler.h"

#include "light.h"


bool cameraUpdated = false;

Camera::Camera()
{
	cameraUpdated = true;
}

void Camera::ProcessInputEvent()
{
	if (possessCamera)
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
		if(windowHandler.mouseMoved == true)
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
			windowHandler.mouseMoved = false;
			cameraUpdated = true;
		}
	}
	if(windowHandler.GetMInput(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		possessCamera = true;
		glfwSetInputMode(windowHandler.p_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		cameraUpdated = true;
	}
	else
	{
		possessCamera = false;
		glfwSetInputMode(windowHandler.p_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	

	inputAxis = glm::clamp(inputAxis, { -1.0,-1.0,-1.0 }, { 1.0,1.0,1.0 });
	if(inputAxis != glm::vec3(0))
	{
		cameraUpdated = true;
	}
}

void Camera::UpdateCamera(float deltaTime)
{
	ProcessInputEvent();

	const float cam_vel = 0.001f + bSprint * 0.01;
	glm::vec3 forward = { 0,0,-cam_vel };
	glm::vec3 right = { cam_vel,0,0 };
	glm::vec3 up = { 0,cam_vel,0 };

	glm::mat4 cam_rot = GetRotationMatrix();

	forward = cam_rot * glm::vec4(forward, 0.f);
	right = cam_rot * glm::vec4(right, 0.f);
	

	velocity = inputAxis.x * forward + inputAxis.y * right + inputAxis.z * up;

	velocity *= 10 * deltaTime;

	if(possessCamera)
	{
		position += velocity;
	}

	if(cameraUpdated == true)
	{
		GPUCameraData camData{};
		camData.viewMatrix = GetViewMatrix();
		camData.projectionMatrix = GetProjectionMatrix();
		camData.projViewMatrix = camData.projectionMatrix * camData.viewMatrix;

		Renderer::UploadSingleUniformDataToShader("camera buffer", camData, true);
		cameraUpdated = false;
	}
	LightManager::SetCamPos(position);
	LightManager::Update();
	
}


glm::mat4 Camera::GetViewMatrix() const
{
	glm::mat4 cam_rot = GetRotationMatrix();

	glm::mat4 view = glm::translate(glm::mat4{ 1 }, position) * cam_rot;

	//we need to invert the camera matrix
	view = glm::inverse(view);

	return view;
}

glm::mat4 Camera::GetProjectionMatrix(bool bReverse /*= false*/, bool flipY /*=true*/) const
{   
    glm::mat4 pro;  
    if (bReverse == true)
    {
        if(Renderer::GetActiveAPI() == AvailableBackends::Vulkan)
        {
            pro = glm::perspective(glm::radians(Fov), (float)VkSwapChainManager::GetSwapchainExtent().width / (float)VkSwapChainManager::GetSwapchainExtent().height, zFar, zNear);
        } 
        if(flipY == true)
        {
            pro[1][1] *= -1;
        }
        return pro;
    }
    else {
        if(Renderer::GetActiveAPI() == AvailableBackends::Vulkan)
        {
            pro = glm::perspective(glm::radians(Fov), (float)VkSwapChainManager::GetSwapchainExtent().width / (float)VkSwapChainManager::GetSwapchainExtent().height, zNear, zFar);
        }
        if(flipY == true)
        {
            pro[1][1] *= -1;
        }
        return pro;
    }
}

glm::mat4 Camera::GetRotationMatrix() const
{
	glm::mat4 yaw_rot = glm::rotate(glm::mat4{ 1 }, yaw, { 0,-1,0 });
	glm::mat4 pitch_rot = glm::rotate(glm::mat4{ yaw_rot }, pitch, { -1,0,0 });

	return pitch_rot;
} 
