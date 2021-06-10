#include "Include/statistics.h"

#include <chrono>


std::chrono::time_point<std::chrono::high_resolution_clock> tStart;
std::chrono::time_point<std::chrono::high_resolution_clock> tEnd;

std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;

float frameCounter = 0;

void Statistics::Start()
{
    tStart = std::chrono::high_resolution_clock::now();
}

void Statistics::End()
{
    frameCounter ++;
    tEnd = std::chrono::high_resolution_clock::now();
    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    deltaTime = tDiff / 1000.0f;

    CalculateFps();
}


//private functions 

void Statistics::CalculateFps()
{
    float fpsTimer = (float)(std::chrono::duration<double, std::milli>(tEnd - lastTimestamp).count());
    if (fpsTimer > 1000.0f)
    {
        Fps = static_cast<uint32_t>((float)frameCounter * (1000.0f / fpsTimer));

        frameCounter = 0;
        lastTimestamp = tEnd;
    }
}