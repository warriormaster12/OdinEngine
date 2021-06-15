#pragma once 

#include <iostream>


struct Component 
{
    virtual void Start() = 0;

    virtual void Update(const float& deltaTime) = 0;

    virtual void Destroy() = 0;
};
