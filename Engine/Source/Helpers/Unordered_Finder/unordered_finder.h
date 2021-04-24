#pragma once 

#include <iostream>

#include <unordered_map>


namespace
{  
    //find unordered_map
    template<typename T>
    T* FindUnorderdMap(const std::string& name, std::unordered_map<std::string, T>& data)
    {
        //search for the object, and return nullpointer if not found
        auto it = data.find(name);
        if (it == data.end()) {
            return nullptr;
        }
        else {
            return &it->second;
        }
    }
}