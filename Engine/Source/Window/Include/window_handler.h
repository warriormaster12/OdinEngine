#pragma once 

#include <iostream>
#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES
#include <GLFW/glfw3.h>



struct Resolution
{
    uint32_t width;
    uint32_t height;
};

class WindowHandler
{
public:
    void CreateWindow(uint32_t width, uint32_t height);
    void DestroyWindow();

    bool WindowShouldClose();
    bool MouseMovedFlag();
    void WindowClose();
    int GetKInput(int key);
    int GetMInput(int button);

    Resolution resolution;
    inline static GLFWwindow* p_window;

    inline static bool frameBufferResized = false;
    inline static bool mouseMoved = false;
    

    inline static float xoffset;
    inline static float yoffset;
private: 
    //window callbacks
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    
};

static WindowHandler windowHandler;