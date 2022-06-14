#include "Renderer.h"

namespace wrengine
{
	bool Renderer::windowShouldClose()
	{
		return m_window.shouldClose();
	}
} // namespace wrengine