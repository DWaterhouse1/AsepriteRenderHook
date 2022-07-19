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
	class Model
	{
	public:

		struct Vertex
		{
			glm::vec2 position;
			glm::vec3 color;
			glm::vec2 uv;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		struct VertexData
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
		};

		Model(
			Device& device,
			const Model::VertexData& vertexData
			);
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