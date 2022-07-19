#include "Texture.h"

#include <stdexcept>

#ifdef NDEBUG
	#define STBI_NO_FALIURE_STRINGS
#else
	#define STB_FALIURE_USERMSG
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

namespace wrengine
{
Texture::~Texture()
{
	if (m_stbiData)
	{
		stbi_image_free(m_stbiData);
		m_stbiData = nullptr;
	}

	vkDestroySampler(m_device.device(), m_textureSampler, nullptr);
	vkDestroyImageView(m_device.device(), m_textureImageView, nullptr);
	vkDestroyImage(m_device.device(), m_textureImage, nullptr);
	vkFreeMemory(m_device.device(), m_textureImageMemory, nullptr);
}

void Texture::loadFromFile(std::string filePath)
{
	if (m_stbiData)
	{
		stbi_image_free(m_stbiData);
		m_stbiData = nullptr;
	}

	m_stbiData = stbi_load(
		filePath.c_str(),
		&m_width,
		&m_height,
		&m_numChannels,
		STBI_rgb_alpha);

	if (stbi_failure_reason())
	{
		throw std::runtime_error(
			"failed to load texture: " +
			std::string(stbi_failure_reason()));
	}

	createTextureBuffer();
}

void Texture::createTextureBuffer()
{
	uint32_t imageSize = m_width * m_height;
	VkDeviceSize pixelSize = 4 * sizeof(unsigned char);
	VkDeviceSize bufferSize = pixelSize * imageSize;

	Buffer stagingBuffer{
		m_device,
		pixelSize,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	stagingBuffer.map();
	stagingBuffer.writeToBuffer(m_stbiData);

	m_textureBuffer = std::make_unique<Buffer>(
		m_device,
		pixelSize,
		imageSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

	m_device.copyBuffer(
		stagingBuffer.getBuffer(),
		m_textureBuffer->getBuffer(),
		bufferSize);

	if (m_stbiData)
	{
		stbi_image_free(m_stbiData);
		m_stbiData = nullptr;
	}

	createImage();

	// transition the image for copying
	transitionImageLayout(
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	m_device.copyBufferToImage(
		stagingBuffer.getBuffer(),
		m_textureImage,
		static_cast<uint32_t>(m_width),
		static_cast<uint32_t>(m_height),
		1);

	// transition back to sate so we can sample from shaders
	transitionImageLayout(
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	createTextureSampler();
}

void Texture::createImage()
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(m_width);
	imageInfo.extent.height = static_cast<uint32_t>(m_height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = 
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // optional

	m_device.createImageWithInfo(
		imageInfo,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_textureImage,
		m_textureImageMemory);
}

void Texture::transitionImageLayout(
	VkFormat format,
	VkImageLayout oldLayout,
	VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = m_device.beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_textureImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	if (
		oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (
		oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,					// src stage mask, dst stage mask
		0,													// dependency flags
		0, nullptr,									// mem barrier count, pMemoryBarriers
		0, nullptr,									// buf barrier count, pBufferBarriers
		1, &barrier									// img barrier count, pImageBarriers
	);

	m_device.endSingleTimeCommands(commandBuffer);
}

void Texture::createImageView()
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(
		m_device.device(),
		&viewInfo,
		nullptr,
		&m_textureImageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}
}

void Texture::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 
		m_device
		.getPhysicalDeviceProperties()
		.limits
		.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(
		m_device.device(),
		&samplerInfo,
		nullptr,
		&m_textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}

} // namespace wrengine