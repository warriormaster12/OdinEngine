#include "Include/window_handler.h"
#include "logger.h"


float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;

bool firstMouse = true;





void WindowHandler::CreateWindow(uint32_t width, uint32_t height)
{
    // We initialize SDL and create a window with it. 
	glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    resolution.width = width;
    resolution.height = height;
	
	p_window = glfwCreateWindow(resolution.width, resolution.height, "Odin Engine", nullptr, nullptr);
	glfwSetWindowUserPointer(p_window, this);
    glfwSetCursorPosCallback(p_window, MouseCallback);
	glfwSetFramebufferSizeCallback(p_window, FramebufferResizeCallback);

    ENGINE_CORE_INFO("window intialized");
}

void WindowHandler::DestroyWindow()
{
    glfwDestroyWindow(p_window);
    glfwTerminate();
    ENGINE_CORE_INFO("window destroyed");
}

int WindowHandler::GetKInput(int key)
{
	return glfwGetKey(p_window, key);
}
int WindowHandler::GetMInput(int button)
{
	return glfwGetMouseButton(p_window, button);
}

bool WindowHandler::WindowShouldClose()
{
	return glfwWindowShouldClose(p_window);
}

void WindowHandler::WindowClose()
{
	glfwSetWindowShouldClose(p_window, true);
}

void WindowHandler::MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    auto app = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    app->xoffset = xoffset;
    app->yoffset = -yoffset;
    
    app->mouseMoved = true;
    lastX = xpos;
    lastY = ypos;
}

void WindowHandler::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));
    app->frameBufferResized = true;
}



