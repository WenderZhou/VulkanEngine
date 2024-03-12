#pragma once

#include "device.h"
#include "buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace VulkanEngine
{
class Model
{
public:

	struct Vertex
	{
		glm::vec3 position{};
		glm::vec3 color{};
		glm::vec3 normal{};
		glm::vec2 texcoord{};
		
		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

		bool operator == (const Vertex& vertex) const
		{
			return position == vertex.position && color == vertex.color && normal == vertex.normal && texcoord == vertex.texcoord;
		}
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		void load(const std::string& filepath);
	};

	Model(Device& device, const Mesh& mesh);
	~Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	static std::unique_ptr<Model> createModelFromFile(Device& device, const std::string& filepath);

	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

private:
	void createVertexBuffers(const std::vector<Vertex>& vertices);
	void createIndexBuffers(const std::vector<uint32_t>& indices);

	Device& m_device;

	std::unique_ptr<Buffer> m_vertexBuffer;
	uint32_t m_vertexCount;

	bool m_hasIndexBuffer = false;
	std::unique_ptr<Buffer> m_indexBuffer;
	uint32_t m_indexCount;
};
}