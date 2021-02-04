#include "Include/camera.h"
#include "SDL.h"

#include <glm/gtx/transform.hpp>

Camera::Camera(vkcomponent::SwapChain& swapChain)
{
    p_swapChain = &swapChain;
}
void Camera::ProcessInputEvent(SDL_Event* p_ev)
{
	if (p_ev->type == SDL_KEYDOWN)
	{
		switch (p_ev->key.keysym.sym)
		{
		case SDLK_UP:
		case SDLK_w:
			inputAxis.x += 1.f;
			break;
		case SDLK_DOWN:
		case SDLK_s:
			inputAxis.x -= 1.f;
			break;
		case SDLK_LEFT:
		case SDLK_a:
			inputAxis.y -= 1.f;
			break;
		case SDLK_RIGHT:
		case SDLK_d:
			inputAxis.y += 1.f;
			break;		
		case SDLK_q:
			inputAxis.z -= 1.f;
			break;
		
		case SDLK_e:
			inputAxis.z += 1.f;
			break;
		case SDLK_LSHIFT:
			bSprint = true;
			break;
		}
	}
	else if (p_ev->type == SDL_KEYUP)
	{
		switch (p_ev->key.keysym.sym)
		{
		case SDLK_UP:
		case SDLK_w:
			inputAxis.x -= 1.f;
			break;
		case SDLK_DOWN:
		case SDLK_s:
			inputAxis.x += 1.f;
			break;
		case SDLK_LEFT:
		case SDLK_a:
			inputAxis.y += 1.f;
			break;
		case SDLK_RIGHT:
		case SDLK_d:
			inputAxis.y -= 1.f;
			break;
		case SDLK_q:
			inputAxis.z += 1.f;
			break;

		case SDLK_e:
			inputAxis.z -= 1.f;
			break;
		case SDLK_LSHIFT:
			bSprint = false;
			break;
		}
	}
	else if (p_ev->type == SDL_MOUSEBUTTONDOWN)
	{
		switch (p_ev->button.button)
		{
			case SDL_BUTTON_RIGHT:
				possessCamera = true;
				break;
		}
	}
	else if (p_ev->type == SDL_MOUSEBUTTONUP)
	{
		switch (p_ev->button.button)
		{
			case SDL_BUTTON_RIGHT:
				possessCamera = false;
				break;
		}
	}
	else if (p_ev->type == SDL_MOUSEMOTION && possessCamera == true) {
		if (!bLocked)
		{
			pitch += p_ev->motion.yrel * 0.003f;
			yaw += p_ev->motion.xrel * 0.003f;

			// 1.56 radians is about equal to 89.x degrees
			if(pitch > 1.56f)
			{
				pitch = 1.56f;
			}
			if(pitch < -1.56f)
			{
				pitch = -1.56f;
			}
		}
	}
	SDL_SetRelativeMouseMode(SDL_bool(possessCamera));
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
	
	if(possessCamera == true)
	{
		velocity = inputAxis.x * forward + inputAxis.y * right + inputAxis.z * up;

		velocity *= 10 * deltaTime;

	}
	else
	{
		velocity = glm::vec3(0.0f);
	}
	position += velocity;
	

}


glm::mat4 Camera::GetViewMatrix()
{
	glm::vec3 camPos = position;

	glm::mat4 cam_rot = (get_rotation_matrix());

	glm::mat4 view = glm::translate(glm::mat4{ 1 }, camPos) * cam_rot;

	//we need to invert the camera matrix
	view = glm::inverse(view);

	return view;
}

glm::mat4 Camera::GetProjectionMatrix(bool bReverse /*= true*/)
{
	if (bReverse)
	{
		glm::mat4 pro = glm::perspective(glm::radians(Fov), (float)p_swapChain->actualExtent.width / (float)p_swapChain->actualExtent.height, zFar, zNear);
		pro[1][1] *= -1;
		return pro;
	}
	else {
		glm::mat4 pro = glm::perspective(glm::radians(Fov), (float)p_swapChain->actualExtent.width / (float)p_swapChain->actualExtent.height, zNear, zFar);
		pro[1][1] *= -1;
		return pro;
	}
}

glm::mat4 Camera::get_rotation_matrix()
{
	glm::mat4 yaw_rot = glm::rotate(glm::mat4{ 1 }, yaw, { 0,-1,0 });
	glm::mat4 pitch_rot = glm::rotate(glm::mat4{ yaw_rot }, pitch, { -1,0,0 });

	return pitch_rot;
} 
