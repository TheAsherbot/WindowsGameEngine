// MipMap.cpp : Defines the entry point for the application.
//

#include "MipMap.h"
#include "Engine/ConsoleRenderer/ConsoleRenderer.h"

int main()
{
	new Renderer::ConsoleRenderer();

	std::cout << "Hello CMake.\n";
	return 0;
}
