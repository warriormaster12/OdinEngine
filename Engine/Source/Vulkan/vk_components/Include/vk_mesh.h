#pragma once

#include "vk_types.h"
#include "logger.h"
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <iostream>


struct VertexInputDescription {

	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct LocationInfo
{
	VkFormat format;
	uint32_t offset;
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;
	static VertexInputDescription GetVertexDescription(const std::vector<LocationInfo>& locations);
};


struct Mesh {
	std::vector<Vertex> vertices;


	AllocatedBuffer vertexBuffer;
	
	bool LoadFromObj(const char* filename);
};

