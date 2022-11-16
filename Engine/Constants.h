#pragma once

#include <vulkan/vulkan.hpp>

#define WR_VULKAN

#ifdef WR_VULKAN
// image filter options
#define WR_FILTER_NEAREST   VK_FILTER_NEAREST
#define WR_FILTER_LINEAR    VK_FILTER_LINEAR
#define WR_FILTER_CUBIC     VK_FILTER_CUBIC_EXT
#define WR_FILTER_CUBIC_IMG VK_FILTER_CUBIC_EXT

#endif // WR_VULKAN

namespace wrengine
{

} // namespace wrengine