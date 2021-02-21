#pragma once 

#include "vk_types.h"
#include <iostream>
#include <functional>
#include <deque>


struct FunctionQueuer
{
    std::deque<std::function<void()>> executer;

    void PushFunction(std::function<void()>&& function) {
        executer.push_back(function);
    }

    void Flush() {
        // reverse iterate the executer queue to execute all the functions
        for (auto it = executer.rbegin(); it != executer.rend(); it++) {
            (*it)(); //call functors
        }

        executer.clear();
    }
};
