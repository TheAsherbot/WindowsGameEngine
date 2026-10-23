// MipMap.cpp : Defines the entry point for the application.
//

#include "MipMap.h"
#include "Engine/Renderer/ConsoleRenderer/ConsoleRenderer.h"
#include "Engine/Input/Win32Input.h"

void Start();
void Update(float deltaTime);

Renderer::ConsoleRenderer* renderer;
Input::InputManager* inputManager;

const POINT* mousePosition;

int main()
{
	HANDLE consoleInputHandle = nullptr;

	renderer = new Renderer::ConsoleRenderer((short)20, (short)8, (short)16, (short)18, &Start, &Update, &consoleInputHandle);
	inputManager = new Input::InputManager(renderer->GetWindowHandlePointer());

	inputManager->RegisterKey(Input::A);

	renderer->Start();

	delete renderer;
	delete inputManager;

	return 0;
}

void Start()
{
	// inputManager->SetMouseTracker((Input::MouseTracker)0);

	
	mousePosition = inputManager->GetMousePositionPointer();
}

void Update(float deltaTime)
{

	inputManager->Update();

	renderer->HideCurser(true);

	if (GetKeyState(VK_LBUTTON) & 0x8000)
	{
		renderer->DrawString(4, 1, "A Pressed");
	}

	renderer->DrawString(1, 2, "                                  ");
	renderer->DrawString(1, 2, std::to_string(mousePosition->x));
}
