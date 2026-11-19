#pragma once
#include "Engine/Renderer/DirectX/DX12Renderer.h"
#include "Engine/Input/Win32Input.h"
#include "Player.h"
#include "Level.h"
#include "Particles.h"

namespace Games
{
	enum class GameState
	{
		MainMenu,
		LevelSelect,
		Controls,
		Playing,
		LevelClear,
		GameComplete,
		Paused
	};

	enum class GameMode
	{
		FullRun,  // For real run: plays through levels 1->5 in sequence, no level skipping
		Practice  // Practice mode: switch between levels 1-5 at any time
	};

	class Game
	{
	public:
		Game(Renderer::DX12Renderer* renderer, Input::InputManager* inputManager);
		~Game() = default;

		void Start();
		void Update(float dt);
		void End();

	private:
		Renderer::DX12Renderer* renderer;
		Input::InputManager* inputManager;

		Player player;
		Level level;
		ParticleSystem particles;

		GameState state = GameState::MainMenu;
		GameMode mode = GameMode::FullRun;

		float totalTime = 0.0f;
		float levelTime = 0.0f;
		int totalDeaths = 0;
		float levelClearTimer = 0.0f;

		// Menu state
		int mainMenuIndex = 0;     // 0: Play, 1: Level Select, 2: Controls, 3: Quit
		int levelSelectIndex = 1;  // 1 to 5
		float menuAnimTimer = 0.0f;

		// Fastest (Gold) times per level (levels 1-5)
		float bestLevelTimes[6] = { 0.0f };
		// Times achieved in the current playthrough / run (levels 1-5)
		float runLevelTimes[6] = { 0.0f };
		bool runIsGold[6] = { false };
		bool isNewGold = false;
		float prevBestTime = 0.0f;

		void LoadRecords();
		void SaveRecords();

		static void DrawCenteredString(Renderer::DX12Renderer* r, int y, const std::string& text, Renderer::Color c, float scale);
		static void DrawCenteredStringInBox(Renderer::DX12Renderer* r, int x1, int x2, int y, const std::string& text, Renderer::Color c, float scale);

		void NextLevel();
		void RestartLevel();
		void JumpToLevel(int index);

		void RenderMainMenu();
		void RenderLevelSelect();
		void RenderControls();
		void RenderHUD();
		void RenderLevelClear();
		void RenderGameComplete();
		void RenderPause();
	};
}
