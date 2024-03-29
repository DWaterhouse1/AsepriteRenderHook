#pragma once

#include "Window.h"

//std
#include <vector>
#include <string>

namespace wrengine
{
/**
* Container for Swapchain support queried from the physical device.
*/
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

/**
* Container for queue family indices, and flags for checking if such indices
* are supported by the physical device.
*/
struct QueueFamilyIndices {
	uint32_t graphicsFamily;
	uint32_t presentFamily;
	bool graphicsFamilyHasValue = false;
	bool presentFamilyHasValue = false;
	bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

/**
* Abstraction over Vulkan physical and logical devices. Contains methods for
* querying required device extensions and parameters. Operates validation layers
* if NDEBUG is not defined, outputting to the standard error stream.
*/
class Device
{
public:
	// enables validation layers only in debug configurations
#ifdef NDEBUG
	static constexpr bool enableValidationLayers = false;
#else
	static constexpr bool enableValidationLayers = true;
#endif

	Device(Window& window);
	~Device();

	// not copyable or movable
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
	Device(Device&&) = delete;
	Device& operator=(Device&&) = delete;

	// getters
	VkCommandPool getCommandPool() { return m_commandPool; }
	VkDevice device() { return m_device; }
	VkSurfaceKHR surface() { return m_surface; }
	VkQueue graphicsQueue() { return m_graphicsQueue; }
	VkQueue presentQueue() { return m_presentQueue; }
	VkInstance getInstance() { return m_instance; }
	VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; }
	uint32_t getGraphicsQueueFamily();
	VkPhysicalDeviceProperties getPhysicalDeviceProperties();
	SwapChainSupportDetails getSwapChainSupport();

	uint32_t findMemoryType(
		uint32_t typeFilter,
		VkMemoryPropertyFlags properties);
	QueueFamilyIndices findPhysicalQueueFamilies();
	VkFormat findSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features);

	// buffer helper functions
	void createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void copyBufferToImage(
		VkBuffer buffer,
		VkImage image,
		uint32_t width,
		uint32_t height,
		uint32_t layerCount);

	void createImageWithInfo(
		const VkImageCreateInfo& imageInfo,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory);

private:
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createCommandPool();

	// helper functions
	bool isDeviceSuitable(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void populateDebugMessengerCreateInfo(
		VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void hasGlfwRequiredInstanceExtensions();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	Window& m_window;
	VkInstance m_instance;
	VkSurfaceKHR m_surface;
	VkDebugUtilsMessengerEXT m_messenger;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkCommandPool m_commandPool;
	VkDevice m_device;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	VkPhysicalDeviceProperties m_properties;

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		"VK_KHR_shader_non_semantic_info"
	};
};
} // namespace wrengine