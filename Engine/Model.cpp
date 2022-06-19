#include "Model.h"

// std
#include <cassert>

namespace wrengine
{
	std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}

	Model::Model(
		Device& device,
		const Model::VertexData& vertexData
	) :
		m_device{ device }
	{
		createVertexBuffers(vertexData.vertices);
		createIndexBuffers(vertexData.indices);
	}

	Model::~Model()
	{
		vkDestroyBuffer(m_device.device(), m_vertexBuffer, nullptr);
		vkFreeMemory(m_device.device(), m_vertexBufferMemory, nullptr);

		if (m_hasIndexBuffer)
		{
			vkDestroyBuffer(m_device.device(), m_indexBuffer, nullptr);
			vkFreeMemory(m_device.device(), m_indexBufferMemory, nullptr);
		}
	}

	void Model::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		
		if (m_hasIndexBuffer)
		{
			vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void Model::draw(VkCommandBuffer commandBuffer)
	{
		m_hasIndexBuffer ?
			vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0) :
			vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
	}

	void Model::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
		m_device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_vertexBuffer,
			m_vertexBufferMemory
			);

		void* data;
		vkMapMemory(m_device.device(), m_vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(m_device.device(), m_vertexBufferMemory);
	}

	void Model::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		m_indexCount = static_cast<uint32_t>(indices.size());
		m_hasIndexBuffer = m_indexCount > 0;
		if (!m_hasIndexBuffer) return;
		VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
		m_device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_indexBuffer,
			m_indexBufferMemory
		);

		void* data;
		vkMapMemory(m_device.device(), m_indexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(m_device.device(), m_indexBufferMemory);
	}
}