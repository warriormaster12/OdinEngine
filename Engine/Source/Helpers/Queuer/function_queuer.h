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


    /**
    *@param inverse if true, functions will be called in order.
    */
    void Flush(const bool& inverse = false) {
        if(inverse == true)
        {
            for (auto it = executer.begin(); it != executer.end(); it++) {
                (*it)(); //call functors
            } 
        }
        else {
            // reverse iterate the executer queue to execute all the functions
           for (auto it = executer.rbegin(); it != executer.rend(); it++) {
                (*it)(); //call functors
            } 
        }
        executer.clear();
    }
};
