// MipMap.cpp : Defines the entry point for the application.
//

#include "MipMap.h"
#include "Engine/Renderer/ConsoleRenderer/ConsoleRenderer.h"

void Start();
void Update(float deltaTime);

Renderer::ConsoleRenderer* renderer;

int main()
{
	HANDLE consoleInputHandle = nullptr;

	renderer = new Renderer::ConsoleRenderer((short)20, (short)8, (short)16, (short)18, &Start, &Update, &consoleInputHandle);
	
	renderer->Start();

	delete renderer;

	return 0;
}

void Start()
{

}

void Update(float deltaTime)
{
	renderer->DrawString(4, 3, "New Project");
	renderer->DrawString(3, 4, "Structure! :D");
}
