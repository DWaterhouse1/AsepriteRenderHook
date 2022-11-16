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

/**
* Loads an image from file into Vulkan texture structures. Will throw a runtime
* error on failure to read the image.
* 
* @param filePath The relative file path to the file containing image data to
* be used to construct the texture.
*/
void Texture::loadFromFile(std::string filePath)
{
	if (m_stbiData)
	{
		stbi_image_free(m_stbiData);
		m_stbiData = nullptr;
	}

	const char* cstrFilePath = filePath.c_str();
	std::cout << cstrFilePath << "\n";

	m_stbiData = stbi_load(
		cstrFilePath,
		&m_width,
		&m_height,
		&m_numChannels,
		STBI_rgb_alpha);

	if (stbi_failure_reason())
	{
		throw std::runtime_error(
			"failed to load texture at " +
			filePath +
			" - " +
			std::string(stbi_failure_reason()));
	}

	createTextureBuffer();
}

/**
* Loads an image from file into Vulkan texture structures and specifies
* configuration options. Will throw a runtime error on failure to read the
* image.
*
* @param filePath The relative file path to the file containing image data to
* be used to construct the texture.
* @param configInfo The struct with specified configuration options.
*/
void Texture::loadFromFile(std::string filePath, TextureConfigInfo configInfo)
{
	m_configInfo = configInfo;
	this->loadFromFile(std::move(filePath));
}

/**
* Loads the data from given data ptr in to Vulkan texture structures. Assumes
* the use of 4 channel R8B8G8A8 data.
* 
* @param data Void ptr of the data to write the texture with.
* @param width The width in pixels of the image data.
* @param height The height in pixels of the image data.
*/
void Texture::loadFromData(
	void* data,
	int width,
	int height)
{
	m_numChannels = 4;
	createTextureBuffer(data, width, height);
}

/**
* Loads the data from given data ptr in to Vulkan texture structures and
* specifies configuration options. Assumes the use of 4 channel R8B8G8A8 data.
*
* @param data Void ptr of the data to write the texture with.
* @param width The width in pixels of the image data.
* @param height The height in pixels of the image data.
*/
void Texture::loadFromData(
	void* data,
	int width,
	int height,
	TextureConfigInfo configInfo)
{
	m_configInfo = configInfo;
	this->loadFromData(data, width, height);
}

/**
* Gets the VkDescriptorImageInfo for use with this Texture.
*/
VkDescriptorImageInfo Texture::descriptorInfo()
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = m_textureImageView;
	imageInfo.sampler = m_textureSampler;
	return imageInfo;
}

/**
* Updates the texture object on device memory to use new data. Supports 256x256
* RGBA8888 data only.
* 
* @param data Pointer to new texture data.
*/
void Texture::updateTextureData(void* data)
{
	// sync by waiting for device
	// TODO this is likely excessive, prefer to syncronise on a GPU fence.
	vkDeviceWaitIdle(m_device.device());

	//TODO fix this to write to the image buffer properly, rather than staging
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
	stagingBuffer.writeToBuffer(data);

	m_device.copyBuffer(
		stagingBuffer.getBuffer(),
		m_textureBuffer->getBuffer(),
		bufferSize);

	transitionImageLayout(
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	m_device.copyBufferToImage(
		stagingBuffer.getBuffer(),
		m_textureImage,
		static_cast<uint32_t>(m_width),
		static_cast<uint32_t>(m_height),
		1);

	// transition back to read only optimal so we can sample from shaders
	transitionImageLayout(
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

/**
* Creates the buffer structures to hold the texture data using class member
* values.
*/
void Texture::createTextureBuffer()
{
	createTextureBuffer(m_stbiData, m_width, m_height);
}

/**
* Creates the buffer structures to hold the texture data.
* 
* @param data Void ptr of the data to write to buffer.
* @param width The width in pixels of the image data.
* @param height The height in pixels of the image data.
*/
void Texture::createTextureBuffer(void* data, int width, int height)
{
	m_width = width;
	m_height = height;
	uint32_t imageSize = width * height;
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
	stagingBuffer.writeToBuffer(data);

	m_textureBuffer = std::make_unique<Buffer>(
		m_device,
		pixelSize,
		imageSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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

	// transition back to read only optimal so we can sample from shaders
	transitionImageLayout(
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	createTextureSampler();
	createImageView();
}

/**
* Creates the texture image.
*/
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

/**
* Transitions the texture image between memory layouts. Will throw a runtime
* error if the layout transition is unsupported.
* 
* @param format Image format.
* @param oldLayout The layout that the image is transitioning from.
* @param newLayout The target image layout that the image is transitioning to.
*/
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
	else if (
		oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
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

/**
* Creates the image view for this texture image. Will throw a runtime error if
* unsuccessful.
*/
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

/**
* Creates the sampler object for using this texutre in shaders. Will throw a
* runtime error if unsuccessful.
*/
void Texture::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = m_configInfo.filterType;
	samplerInfo.minFilter = m_configInfo.filterType;
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