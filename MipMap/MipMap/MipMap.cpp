#include "Engine/Renderer/DirectX/DX12Renderer.h"
#include "Engine/Input/Win32Input.h"
#include "Games/Game.h"
#include <iostream>

Renderer::DX12Renderer* renderer = nullptr;
Input::InputManager* inputManager = nullptr;
Games::Game* game = nullptr;

void Start()
{
    game = new Games::Game(renderer, inputManager);
    game->Start();
}

void Update(float deltaTime)
{
    inputManager->Update();
    game->Update(deltaTime);
}

int main()
{
    try
    {
        // 1. Create DX12Renderer with 1280x720 window
        renderer = new Renderer::DX12Renderer(1280, 720, &Start, &Update, "Cube Dash");

        // 2. Direct drop-in with Win32Input
        inputManager = new Input::InputManager(renderer->GetWindowHandlePointer());

        // 3. Run game loop
        renderer->Start();

        delete game;
        delete inputManager;
        delete renderer;
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }
}
