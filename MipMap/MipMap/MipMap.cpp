#include "Engine/Renderer/DirectX/DX12Renderer.h"
#include "Engine/Input/Win32Input.h"
#include <iostream>

Renderer::DX12Renderer* renderer = nullptr;
Input::InputManager* inputManager = nullptr;
Renderer::Texture* myTexture = nullptr;

void Start()
{
    // Configure input tracker exactly as in your console setup
    inputManager->SetMouseTracker(static_cast<Input::MouseTracker>(Input::WindowSelected | Input::OverWindows));
    inputManager->RegisterKey(Input::A);
    inputManager->RegisterKey(Input::D);
    inputManager->RegisterKey(Input::W);
    inputManager->RegisterKey(Input::S);
    inputManager->RegisterKey(Input::Esc);

    // Optional: Load texture from file (PNG, JPG, BMP)
     myTexture = renderer->LoadTexture("Assets/player.");
}

void Update(float deltaTime)
{
    inputManager->Update();

    // 1. Clear background
    renderer->Clear(Renderer::Color(0.08f, 0.08f, 0.12f, 1.0f));

    // 2. Render Text with 8x8 font
    renderer->DrawString(20, 20, "FINALLY!!! 2D DirectX 12! <3", Renderer::Color::White, 2.0f);
    renderer->DrawString(20, 50, "DeltaTime: " + std::to_string(deltaTime), Renderer::Color::Yellow);

    // 3. Render Primitives
    renderer->DrawFilledRectangle(20, 90, 200, 150, Renderer::Color::Blue);
    renderer->DrawRectangle(20, 90, 200, 150, Renderer::Color::White);
    z

    // 4
    // . Render Texture (if loaded)
    if (myTexture)
    {
     
        //renderer->DrawTexture(250, 90, myTexture);
    }

    // 5. Mouse Interaction directly through Win32Input
    const POINT* mousePos = inputManager->GetMousePositionPointer();
    renderer->DrawString(20, 170, "Mouse X: " + std::to_string(mousePos->x) + " Y: " + std::to_string(mousePos->y), Renderer::Color::Cyan);

    // Draw cursor box tracking mouse
    renderer->DrawFilledRectangle(mousePos->x - 4, mousePos->y - 4, mousePos->x + 4, mousePos->y + 4, Renderer::Color::Red);

    if (inputManager->GetMouseButtonState(Input::Left) == Input::Pressed)
    {
        renderer->DrawString(20, 200, "Left Mouse Pressed!", Renderer::Color::Green);
    }
}



int main()
{
    try
    {
        // 1. Create DX12Renderer
        renderer = new Renderer::DX12Renderer(1280, 720, &Start, &Update, "2D DirectX 12 Game Engine");

        // 2. Direct drop-in with your Win32Input!
        inputManager = new Input::InputManager(renderer->GetWindowHandlePointer());

        // 3. Run game loop
        renderer->Start();

        delete renderer;
        delete inputManager;
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Fatal error: " << error.what() << '\n';

        return 1;
 
    }


