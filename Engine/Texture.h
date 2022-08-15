#pragma once

#include <vulkan/vulkan.hpp>

#include "Device.h"
#include "Buffer.h"

// std
#include <string>
#include <memory>

namespace wrengine
{
class Texture
{
public:
	Texture(Device& device) : m_device{ device } {}
	~Texture();

	// not copyable
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	void loadFromFile(std::string filePath);
	VkDescriptorImageInfo descriptorInfo();

private:
	void createTextureBuffer();
	void createImage();
	void transitionImageLayout(
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout);
	void createImageView();
	void createTextureSampler();

	Device& m_device;
	std::unique_ptr<Buffer> m_textureBuffer;
	VkImage m_textureImage;
	VkDeviceMemory m_textureImageMemory;
	VkImageView m_textureImageView{};
	VkSampler m_textureSampler;
	
	// stbi
	void* m_stbiData = nullptr;
	int m_width{};
  int m_height{};
	int m_numChannels{};
};
} // namespace wrengine