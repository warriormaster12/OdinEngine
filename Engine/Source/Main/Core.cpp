#include "Include/Core.h"
#include "../Vulkan/Include/vk_renderer.h"

bool _isInitialized{ false };
VulkanRenderer renderer;
void Core::coreInit()
{
    renderer.init();
    //everything went fine
    _isInitialized = true;
}

void Core::coreUpdate()
{
    renderer.run();
}

void Core::coreCleanup()
{
    if (_isInitialized)
    {
        renderer.cleanup();
    }
}