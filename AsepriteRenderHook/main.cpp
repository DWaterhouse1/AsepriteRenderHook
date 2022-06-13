#include "AsepriteRenderHook.h"

// std
#include <stdexcept>

int main()
{
	try
	{
		AsepriteRenderHook app{};
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		std::cin.get();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}