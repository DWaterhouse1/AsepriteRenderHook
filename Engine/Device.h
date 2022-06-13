#pragma once

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
	private:
	};
}