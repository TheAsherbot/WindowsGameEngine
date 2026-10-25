// MipMap.cpp : Defines the entry point for the application.
//

#include "MipMap.h"
#include "Engine/Renderer/ConsoleRenderer/ConsoleRenderer.h"
#include "Engine/Input/Win32Input.h"

#include "Games/Celeste.h"

void Start();
void Update(float deltaTime);

Renderer::ConsoleRenderer* renderer;
Input::InputManager* inputManager;

const POINT* mousePosition;

int main()
{
	/*
	HANDLE consoleInputHandle = nullptr;

	renderer = new Renderer::ConsoleRenderer((short)20, (short)8, (short)16, (short)18, &Start, &Update);
	inputManager = new Input::InputManager(renderer->GetWindowHandlePointer());

	inputManager->RegisterKey(Input::A);

	renderer->Start();

	delete renderer;
	delete inputManager;
	*/

	Games::Celeste::Celeste* celste = new Games::Celeste::Celeste();

	

	return 0;
}

void Start()
{
	inputManager->SetMouseTracker((Input::MouseTracker)3);

	
	mousePosition = inputManager->GetMousePositionPointer();
}

int i = 0;
void Update(float deltaTime)
{

	inputManager->Update();
	
	renderer->HideCurser(true);

	if (inputManager->GetMouseButtonState(Input::Left) == Input::Pressed)
	{
		renderer->DrawString(4, 1, "Left Pressed");
	}
	else if (inputManager->GetMouseButtonState(Input::Left) == Input::Held)
	{
		renderer->DrawString(4, 1, "Left Held");
	}
	else if (inputManager->GetMouseButtonState(Input::Left) == Input::Released)
	{
		renderer->DrawString(4, 1, "Left Released");
	}
	else if (inputManager->GetMouseButtonState(Input::Middle) == Input::Pressed)
	{
		renderer->DrawString(4, 1, "Middle Pressed");
	}
	else if (inputManager->GetMouseButtonState(Input::Middle) == Input::Held)
	{
		renderer->DrawString(4, 1, "Middle Held");
	}
	else if (inputManager->GetMouseButtonState(Input::Middle) == Input::Released)
	{
		renderer->DrawString(4, 1, "Middle Released");
	}
	else if (inputManager->GetMouseButtonState(Input::Right) == Input::Pressed)
	{
		renderer->DrawString(4, 1, "Right Pressed");
	}
	else if (inputManager->GetMouseButtonState(Input::Right) == Input::Held)
	{
		renderer->DrawString(4, 1, "Right Held");
	}
	else if (inputManager->GetMouseButtonState(Input::Right) == Input::Released)
	{
		renderer->DrawString(4, 1, "Right Released");
	}
	else if (inputManager->GetMouseButtonState(Input::Side1) == Input::Pressed)
	{
		renderer->DrawString(4, 1, "Side1 Pressed");
	}
	else if (inputManager->GetMouseButtonState(Input::Side1) == Input::Held)
	{
		renderer->DrawString(4, 1, "Side1 Held");
	}
	else if (inputManager->GetMouseButtonState(Input::Side1) == Input::Released)
	{
		renderer->DrawString(4, 1, "Side1 Released");
	}
	else if (inputManager->GetMouseButtonState(Input::Side2) == Input::Pressed)
	{
		renderer->DrawString(4, 1, "Side2 Pressed");
	}
	else if (inputManager->GetMouseButtonState(Input::Side2) == Input::Held)
	{
		renderer->DrawString(4, 1, "Side2 Held");
	}
	else if (inputManager->GetMouseButtonState(Input::Side2) == Input::Released)
	{
		renderer->DrawString(4, 1, "Side2 Released");
	}
	else
	{
		i++;
		if (i > 10000)
		{
			renderer->DrawString(4, 1, "                          ");
			renderer->DrawString(4, 5, "        ");
			renderer->DrawString(4, 5, std::to_string(i));
			renderer->DrawString(1, 2, "X:     Y:                             ");
			renderer->DrawString(4, 2, std::to_string(mousePosition->x));
			renderer->DrawString(11, 2, std::to_string(mousePosition->y));
		}
		return;
	}


	i = 0;

	renderer->DrawString(4, 5, "        ");
	renderer->DrawString(4, 5, std::to_string(i));
	renderer->DrawString(1, 2, "X:     Y:                             ");
	renderer->DrawString(4, 2, std::to_string(mousePosition->x));
	renderer->DrawString(11, 2, std::to_string(mousePosition->y));
	
}
