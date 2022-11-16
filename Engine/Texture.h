#pragma once

#include <vulkan/vulkan.hpp>

#include "Device.h"
#include "Buffer.h"
#include "Constants.h"

// std
#include <string>
#include <memory>

namespace wrengine
{
/**
* Struct for texture configuration options.
*/
struct TextureConfigInfo
{
	VkFilter filterType = VK_FILTER_LINEAR;
};

/**
* Encapsulation of texture resources. Can read images from file and load into
* the Vulkan image and image memory structures. Provides image views and texture
* samplers for use of the texture within a shader.
*/
class Texture
{
public:
	Texture(Device& device) : m_device{ device } {}
	~Texture();

	// not copyable
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	void loadFromFile(std::string filePath);
	void loadFromFile(std::string filePath, TextureConfigInfo configInfo);
	void loadFromData(
		void* data,
		int width,
		int height);
	void loadFromData(
		void* data,
		int width,
		int height,
		TextureConfigInfo configInfo);
	VkDescriptorImageInfo descriptorInfo();
	void updateTextureData(void* data);

private:
	void createTextureBuffer();
	void createTextureBuffer(void* data, int width, int height);
	void createImage();
	void transitionImageLayout(
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout);
	void createImageView();
	void createTextureSampler();

	Device& m_device;
	std::unique_ptr<Buffer> m_textureBuffer;
	VkImage m_textureImage = nullptr;
	VkDeviceMemory m_textureImageMemory = nullptr;
	VkImageView m_textureImageView = nullptr;
	VkSampler m_textureSampler = nullptr;
	
	// stbi
	void* m_stbiData = nullptr;
	int m_width{};
  int m_height{};
	int m_numChannels{};

	// config options
	TextureConfigInfo m_configInfo{};
};
} // namespace wrengine