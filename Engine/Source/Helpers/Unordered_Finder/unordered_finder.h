#pragma once 

#include "logger.h"
#include <iostream>

#include <unordered_map>


namespace
{  
    //find unordered_map
    template<typename M, typename G,typename T>
    T* FindUnorderedMap(const M& name, std::unordered_map<G, T>& data)
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