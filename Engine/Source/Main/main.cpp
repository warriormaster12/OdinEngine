#include "../Vulkan/Include/vk_renderer.h"

int main(int argc, char* argv[])
{
	VulkanRenderer renderer;

	renderer.init();

	renderer.run();

	renderer.cleanup();

	return 0;
}


