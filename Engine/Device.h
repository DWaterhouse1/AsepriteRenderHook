#pragma once

#include "Window.h"

namespace wrengine
{
	class Device
	{
	public:
		// enables validation layers only in debug configurations
#ifdef NDEBUG
		constexpr bool enableValidationLayers = false;
#else
		static constexpr bool enableValidationLayers = true;
#endif

		Device();

	private:
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();
	};
}