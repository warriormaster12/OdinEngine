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
    void createWindow(uint32_t width, uint32_t height);
    void destroyWindow();

    Resolution _resolution;
    struct SDL_Window* _window{ nullptr };
};