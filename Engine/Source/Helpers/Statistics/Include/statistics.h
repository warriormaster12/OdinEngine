#pragma once

#include <iostream>


class Statistics
{
public:
    static void Start();
    static void End();

    static float& GetDeltaTime() {return deltaTime;}
    static float& GetFps() {return Fps;}
private:
    static inline float deltaTime = 0;
    static inline float Fps = 0;

    static void CalculateFps();
};