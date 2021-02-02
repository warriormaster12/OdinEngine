#pragma once 

#include <iostream>
#include <SDL.h>
#include <SDL_vulkan.h>



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

    Resolution resolution;
    struct SDL_Window* window{ nullptr };
};