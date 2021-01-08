#pragma once 
#include "../../../Logger/Include/Logger.h"

namespace vkcomponent
{
    //we want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state.
    using namespace std;
    #define VK_CHECK(x)                                                 \
        do                                                              \
        {                                                               \
            VkResult err = x;                                           \
            if (err)                                                    \
            {                                                           \
                Logger::GetCoreLogger()->error("Detected Vulkan error: {0}",err); \
                abort();                                                \
            }                                                           \
        } while (0)
}