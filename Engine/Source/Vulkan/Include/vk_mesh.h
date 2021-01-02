#pragma once

#include "vk_types.h"
#include <vector>
#include <glm/vec3.hpp>

struct VertexInputDescription {

	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};


struct Vertex {

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    static VertexInputDescription get_vertex_description();
};


struct Mesh {
	std::vector<Vertex> _vertices;

	AllocatedBuffer _vertexBuffer;
};

