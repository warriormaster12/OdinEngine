#include "Include/vk_meshhandler.h"




VertexInputDescription Vertex::GetVertexDescription(const std::vector<LocationInfo>& locations)
{
	VertexInputDescription description;
	//we will have just 1 vertex buffer binding, with a per-vertex rate
	VkVertexInputBindingDescription mainBinding = {};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(Vertex);
	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(mainBinding);

	
	VkVertexInputAttributeDescription attributes[locations.size()];
	for(int i = 0; i < locations.size(); i++)
	{
		attributes[i].binding = 0;
		attributes[i].location = i;
		attributes[i].format = locations[i].format;
		attributes[i].offset = locations[i].offset;

		description.attributes.push_back(attributes[i]);
	}
	return description;
}
