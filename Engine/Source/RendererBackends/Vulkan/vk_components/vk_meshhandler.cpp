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

	
	for(size_t i = 0; i < locations.size(); i++)
	{
		VkVertexInputAttributeDescription attribute{};
		attribute.binding = 0;
		attribute.location = i;
		attribute.format = locations[i].format;
		attribute.offset = locations[i].offset;

		description.attributes.push_back(attribute);
	}
	return description;
}
