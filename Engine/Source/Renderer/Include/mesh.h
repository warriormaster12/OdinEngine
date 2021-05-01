#pragma once 

#include <vector>
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtx/hash.hpp>

#include "vk_types.h"


struct GPUObjectData
{
    glm::mat4 modelMatrix;
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;
	bool operator==(const Vertex& other) const {
		return position == other.position && color == other.color && uv == other.uv;
	}
};



namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            size_t h1 = hash<glm::vec3>()(vertex.position);
            size_t h2 = hash<glm::vec3>()(vertex.color);
            size_t h3 = hash<glm::vec2>()(vertex.uv);
            size_t h4 = hash<glm::vec3>()(vertex.normal);

            return((((h1 ^ (h2 << 1)) >> 1) ^ h3) << 1) ^ h4;
        }
    };
}



struct Mesh
{
    AllocatedBuffer indexBuffer;
    AllocatedBuffer vertexBuffer;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    void CreateMesh();

    void DestroyMesh();

    void LoadFromObj(const std::string& path);
};