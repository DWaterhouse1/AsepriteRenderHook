#include "Device.h"

namespace wrengine
{
	Device::Device()
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}
	void Device::createInstance()
	{
	}
	void Device::setupDebugMessenger()
	{
	}
	void Device::createSurface() {}

	void Device::pickPhysicalDevice()
	{
	}
	void Device::createLogicalDevice()
	{
	}
	void Device::createCommandPool()
	{
	}
}