#pragma once

#include "Device.h"

// vulkan
#include <vulkan/vulkan.hpp>

// std

namespace wrengine
{
	class Swapchain
	{
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		Swapchain(Device &deviceRef, VkExtent2D extent);
		~Swapchain();

		// should not be copied
		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;

		// getters
		VkFramebuffer getFrameBuffer(int index) { return m_swapchainFramebuffers[index]; }
		VkRenderPass getRenderPass() { return m_renderPass; }
		VkImageView getImageView(int index) { return m_imageViews[index]; }
		size_t imageCount() { return m_swapchainImages.size(); }
		VkFormat getSwapChainImageFormat() { return m_swapchainImageFormat; }
		VkExtent2D getSwapChainExtent() { return m_swapchainExtent; }
		uint32_t width() { return m_swapchainExtent.width; }
		uint32_t height() { return m_swapchainExtent.height; }

		// interface functions
		VkFormat findDepthFormat();
		float extentAspectRatio();
		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
		
	private:
		void createSwapChain();
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		// helpers
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		// member vars
		Device& m_device;
		VkExtent2D m_extent;
		VkSwapchainKHR m_swapchain;
		VkFormat m_swapchainImageFormat;
		VkExtent2D m_swapchainExtent;
		VkRenderPass m_renderPass;

		std::vector<VkImage> m_swapchainImages;
		std::vector<VkImageView> m_imageViews;
		std::vector<VkImage> m_depthImages;
		std::vector<VkDeviceMemory> m_depthImageMemorys;
		std::vector<VkImageView> m_depthImageViews;
		std::vector<VkFramebuffer> m_swapchainFramebuffers;

		// sync structures
		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;
		size_t m_currentFrame = 0;
	};
}