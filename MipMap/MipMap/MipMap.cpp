// MipMap.cpp : Defines the entry point for the application.
//

#include "MipMap.h"
#include "Engine/Renderer/ConsoleRenderer/ConsoleRenderer.h"
#include "Engine/Input/Win32Input.h"

void Start();
void Update(float deltaTime);

Renderer::ConsoleRenderer* renderer;
Input::InputManager* inputManager;

int main()
{
	HANDLE consoleInputHandle = nullptr;

	renderer = new Renderer::ConsoleRenderer((short)20, (short)8, (short)16, (short)18, &Start, &Update, &consoleInputHandle);
	inputManager = new Input::InputManager();

	renderer->Start();

	delete renderer;
	delete inputManager;

	return 0;
}

void Start()
{

}

void Update(float deltaTime)
{
	if (inputManager->IsKeyPressed())
	{
		renderer->DrawString(4, 3, "New Project");
		renderer->DrawString(3, 4, "Structure! :D");
	}
}
