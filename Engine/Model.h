#pragma once

#include "Device.h"
#include "Buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>
#include <memory>

namespace wrengine
{
/**
* Represents geometric render data. Owns via unique ptr vertex and optionally
* index buffers. Contains functionality to bind a pipeline object and be drawn
* by the renderer.
*/
class Model
{
public:

	/**
	* Represents and individual vertex. Contains 2D postition, RGB colour and
	* texture UV coordinates.
	*/
	struct Vertex
	{
		glm::vec2 position;
		glm::vec3 color;
		glm::vec2 uv;

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	};

	/**
	* Containts all vertex data for a model object. Indices are optional.
	*/
	struct VertexData
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
	};

	Model(
		Device& device,
		const Model::VertexData& vertexData);
	~Model();

	// should not be copied
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

private:
	void createVertexBuffers(const std::vector<Vertex>& vertices);
	void createIndexBuffers(const std::vector<uint32_t>& indices);

	Device& m_device;

	// vertex data objects
	std::unique_ptr<Buffer> m_vertexBuffer;
	uint32_t m_vertexCount;

	// index data objects
	bool m_hasIndexBuffer = false;
	std::unique_ptr<Buffer> m_indexBuffer;
	uint32_t m_indexCount;
};
}