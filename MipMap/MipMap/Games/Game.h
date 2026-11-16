#include "Engine/Renderer/DirectX/DX12Renderer.h"
#include "Engine/Input/Win32Input.h"

namespace Games
{
	class Game
	{
	public:
		Game(Renderer::DX12Renderer* renderer, Input::InputManager* inputManager);

		void Start();

	private:
		Renderer::DX12Renderer* renderer;
		float deltaTime;

		Input::InputManager* inputManager;

		void Update();
		void End();
	};
}