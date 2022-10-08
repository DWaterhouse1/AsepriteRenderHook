#include "Model.h"

// std
#include <cassert>

namespace wrengine
{
/**
* Gets the binding descriptions of the Vertex type.
* 
* @return Vector of vertex binding descriptions.
*/
std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

/**
* Gets the attribute descriptions of the Vertex type. Vertex data is
* interleaved.
* 
* @return Vector of vertex attribute descriptions.
*/
std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions()
{
	// TODO mode switching 2d/3d for vec2/vec3 position attributes
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, uv);

	return attributeDescriptions;
}

Model::Model(
	Device& device,
	const Model::VertexData& vertexData) :
	m_device{ device }
{
	createVertexBuffers(vertexData.vertices);
	createIndexBuffers(vertexData.indices);
}

Model::~Model() {}

/**
* Binds the vertex buffer ready for drawing. Will bind index buffer if in use.
* 
* @param commandBuffer The command buffer to which the bind commands are
* recorded.
*/
void Model::bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = { m_vertexBuffer->getBuffer()};
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	
	if (m_hasIndexBuffer)
	{
		vkCmdBindIndexBuffer(
			commandBuffer,
			m_indexBuffer->getBuffer(),
			0,
			VK_INDEX_TYPE_UINT32);
	}
}

/**
* Draws the vertex data. Will use an indexed draw if index buffer is in use.
* 
* @param commandBuffer The command buffer to which the draw commands are
* recorded.
*/
void Model::draw(VkCommandBuffer commandBuffer)
{
	m_hasIndexBuffer ?
		vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0) :
		vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
}

/**
* Create the vertex Buffer object and fills it with provided vertex data. Uses
* a staging buffer to transfer, so the final vertex buffer is in device local
* memory.
* 
* @param vertices The vertex data which will fill the created buffer.
*/
void Model::createVertexBuffers(const std::vector<Vertex>& vertices)
{
	m_vertexCount = static_cast<uint32_t>(vertices.size());
	assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
	VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
	uint32_t vertexSize = sizeof(vertices[0]);

	Buffer stagingBuffer{
		m_device,
		vertexSize,
		m_vertexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	stagingBuffer.map();
	stagingBuffer.writeToBuffer((void*)vertices.data());

	m_vertexBuffer = std::make_unique<Buffer>(
		m_device,
		vertexSize,
		m_vertexCount,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	m_device.copyBuffer(
		stagingBuffer.getBuffer(),
		m_vertexBuffer->getBuffer(),
		bufferSize);
}

/**
* Create the index Buffer object and fills it with provided vertex data. Uses
* a staging buffer to transfer, so the final index buffer is in device local
* memory.
*
* @param vertices The index data which will fill the created buffer.
*/
void Model::createIndexBuffers(const std::vector<uint32_t>& indices)
{
	m_indexCount = static_cast<uint32_t>(indices.size());
	m_hasIndexBuffer = m_indexCount > 0;
	if (!m_hasIndexBuffer) return;
	VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
	uint32_t indexSize = sizeof(indices[0]);

	Buffer stagingBuffer{
		m_device,
		indexSize,
		m_indexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	stagingBuffer.map();
	stagingBuffer.writeToBuffer((void*)indices.data());

	m_indexBuffer = std::make_unique<Buffer>(
		m_device,
		indexSize,
		m_indexCount,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	m_device.copyBuffer(
		stagingBuffer.getBuffer(),
		m_indexBuffer->getBuffer(),
		bufferSize);
}
}