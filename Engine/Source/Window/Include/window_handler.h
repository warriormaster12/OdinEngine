#pragma once 

#include <iostream>
#define GLFW_INCLUDE_VULKAN
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
    void WindowClose();
    int GetKInput(int key);
    int GetMInput(int button);

    Resolution resolution;
    GLFWwindow* p_window;

    bool frameBufferResized = false;
    bool mouseMotion = false;

    float xoffset;
    float yoffset;

};