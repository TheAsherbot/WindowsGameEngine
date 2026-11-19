#include "Game.h"
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <windows.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace Games
{
	static std::string FormatTime(float seconds)
	{
		int mins = static_cast<int>(seconds) / 60;
		int secs = static_cast<int>(seconds) % 60;
		int hundredths = static_cast<int>((seconds - static_cast<float>(mins * 60 + secs)) * 100.0f);
		if (hundredths < 0) hundredths = 0;
		if (hundredths > 99) hundredths = 99;

		std::ostringstream ss;
		if (mins < 10) ss << "0";
		ss << mins << ":";
		if (secs < 10) ss << "0";
		ss << secs << ".";
		if (hundredths < 10) ss << "0";
		ss << hundredths;
		return ss.str();
	}

	Game::Game(Renderer::DX12Renderer* renderer, Input::InputManager* inputManager)
		: renderer(renderer), inputManager(inputManager)
	{
	}

	void Game::LoadRecords()
	{
		for (int i = 0; i <= 5; ++i) bestLevelTimes[i] = 0.0f;
		std::ifstream file("records.txt");
		if (file.is_open())
		{
			for (int i = 1; i <= 5; ++i)
			{
				float t = 0.0f;
				if (file >> t)
				{
					bestLevelTimes[i] = t;
				}
			}
		}
	}

	void Game::SaveRecords()
	{
		std::ofstream file("records.txt");
		if (file.is_open())
		{
			for (int i = 1; i <= 5; ++i)
			{
				file << bestLevelTimes[i] << "\n";
			}
		}
	}

	void Game::DrawCenteredString(Renderer::DX12Renderer* r, int y, const std::string& text, Renderer::Color c, float scale)
	{
		int charWidth = static_cast<int>(8.0f * scale);
		int textWidth = static_cast<int>(text.length()) * charWidth;
		int x = (1280 - textWidth) / 2;
		r->DrawString(x, y, text, c, scale);
	}

	void Game::DrawCenteredStringInBox(Renderer::DX12Renderer* r, int x1, int x2, int y, const std::string& text, Renderer::Color c, float scale)
	{
		int charWidth = static_cast<int>(8.0f * scale);
		int textWidth = static_cast<int>(text.length()) * charWidth;
		int boxWidth = x2 - x1;
		int x = x1 + (boxWidth - textWidth) / 2;
		r->DrawString(x, y, text, c, scale);
	}

	void Game::Start()
	{
		LoadRecords();

		// Configure mouse tracking
		inputManager->SetMouseTracker(static_cast<Input::MouseTracker>(Input::WindowSelected | Input::OverWindows));

		// Movement Keys
		inputManager->RegisterKey(Input::A);
		inputManager->RegisterKey(Input::D);
		inputManager->RegisterKey(Input::W);
		inputManager->RegisterKey(Input::S);
		inputManager->RegisterKey(Input::LeftArrow);
		inputManager->RegisterKey(Input::RightArrow);
		inputManager->RegisterKey(Input::UpArrow);
		inputManager->RegisterKey(Input::DownArrow);

		// Jump Keys
		inputManager->RegisterKey(Input::Space);
		inputManager->RegisterKey(Input::C);
		inputManager->RegisterKey(Input::Z);

		// Dash Keys
		inputManager->RegisterKey(Input::J);
		inputManager->RegisterKey(Input::X);
		inputManager->RegisterKey(Input::K);
		inputManager->RegisterKey(Input::Shift);
		inputManager->RegisterKey(Input::LShift);

		// Control & Utility Keys
		inputManager->RegisterKey(Input::R);
		inputManager->RegisterKey(Input::Esc);
		inputManager->RegisterKey(Input::One);
		inputManager->RegisterKey(Input::Two);
		inputManager->RegisterKey(Input::Three);
		inputManager->RegisterKey(Input::Four);
		inputManager->RegisterKey(Input::Five);
		inputManager->RegisterKey(Input::Enter);
		inputManager->RegisterKey(Input::M);

		// Initialize state to MainMenu when first entering the game!
		state = GameState::MainMenu;
		mode = GameMode::FullRun;
		mainMenuIndex = 0;
		levelSelectIndex = 1;
		menuAnimTimer = 0.0f;
		totalTime = 0.0f;
		levelTime = 0.0f;
		totalDeaths = 0;
		for (int i = 0; i <= 5; ++i)
		{
			runLevelTimes[i] = 0.0f;
			runIsGold[i] = false;
		}

		// Preload Level 1 & Spawn Player
		level.LoadLevel(1);
		player.Spawn(level.GetSpawnX(), level.GetSpawnY());
	}

	void Game::Update(float dt)
	{
		// Cap deltaTime to avoid instability on window moves or lag spikes
		if (dt > 0.05f) dt = 0.05f;
		menuAnimTimer += dt;

		// -------------------------------------------------------------
		// 1. MAIN MENU STATE
		// -------------------------------------------------------------
		if (state == GameState::MainMenu)
		{
			particles.Update(dt);

			// Gentle ambient sparkles in the menu background
			if (rand() % 10 == 0)
			{
				float sx = static_cast<float>(rand() % 1280);
				float sy = static_cast<float>(rand() % 720);
				particles.SpawnPortalSparkles(sx, sy, 18.0f);
			}

			// Menu Navigation: W / S or Up / Down
			if (inputManager->GetRegisteredKeyState(Input::W) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::UpArrow) == Input::Pressed)
			{
				mainMenuIndex = (mainMenuIndex + 3) % 4; // 0: Full Run, 1: Practice, 2: Controls, 3: Quit
			}
			if (inputManager->GetRegisteredKeyState(Input::S) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::DownArrow) == Input::Pressed)
			{
				mainMenuIndex = (mainMenuIndex + 1) % 4;
			}

			// Activation: Space or Enter
			if (inputManager->GetRegisteredKeyState(Input::Space) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::Enter) == Input::Pressed)
			{
				if (mainMenuIndex == 0)
				{
					// For Real Run: Start Full Run from Level 1
					mode = GameMode::FullRun;
					level.LoadLevel(1);
					player.Spawn(level.GetSpawnX(), level.GetSpawnY());
					particles.Clear();
					totalTime = 0.0f;
					levelTime = 0.0f;
					totalDeaths = 0;
					for (int i = 0; i <= 5; ++i)
					{
						runLevelTimes[i] = 0.0f;
						runIsGold[i] = false;
					}
					state = GameState::Playing;
				}
				else if (mainMenuIndex == 1)
				{
					// Practice Mode: Open Level Select (Can jump between 1-5 anytime)
					mode = GameMode::Practice;
					state = GameState::LevelSelect;
					levelSelectIndex = 1;
				}
				else if (mainMenuIndex == 2)
				{
					state = GameState::Controls;
				}
				else if (mainMenuIndex == 3)
				{
					PostQuitMessage(0);
				}
			}

			// Render Main Menu
			renderer->Clear(Renderer::Color(0.04f, 0.05f, 0.09f, 1.0f));
			particles.Render(renderer);
			RenderMainMenu();
			return;
		}

		// -------------------------------------------------------------
		// 2. LEVEL SELECT STATE
		// -------------------------------------------------------------
		if (state == GameState::LevelSelect)
		{
			particles.Update(dt);

			if (inputManager->GetRegisteredKeyState(Input::A) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::LeftArrow) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::W) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::UpArrow) == Input::Pressed)
			{
				levelSelectIndex = (levelSelectIndex > 1) ? (levelSelectIndex - 1) : 5;
			}
			if (inputManager->GetRegisteredKeyState(Input::D) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::RightArrow) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::S) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::DownArrow) == Input::Pressed)
			{
				levelSelectIndex = (levelSelectIndex < 5) ? (levelSelectIndex + 1) : 1;
			}

			if (inputManager->GetRegisteredKeyState(Input::One) == Input::Pressed) levelSelectIndex = 1;
			if (inputManager->GetRegisteredKeyState(Input::Two) == Input::Pressed) levelSelectIndex = 2;
			if (inputManager->GetRegisteredKeyState(Input::Three) == Input::Pressed) levelSelectIndex = 3;
			if (inputManager->GetRegisteredKeyState(Input::Four) == Input::Pressed) levelSelectIndex = 4;
			if (inputManager->GetRegisteredKeyState(Input::Five) == Input::Pressed) levelSelectIndex = 5;

			if (inputManager->GetRegisteredKeyState(Input::Space) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::Enter) == Input::Pressed)
			{
				JumpToLevel(levelSelectIndex);
				state = GameState::Playing;
			}

			if (inputManager->GetRegisteredKeyState(Input::Esc) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::M) == Input::Pressed)
			{
				state = GameState::MainMenu;
			}

			renderer->Clear(Renderer::Color(0.04f, 0.05f, 0.09f, 1.0f));
			particles.Render(renderer);
			RenderLevelSelect();
			return;
		}

		// -------------------------------------------------------------
		// 3. CONTROLS STATE
		// -------------------------------------------------------------
		if (state == GameState::Controls)
		{
			particles.Update(dt);

			if (inputManager->GetRegisteredKeyState(Input::Space) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::Enter) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::Esc) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::M) == Input::Pressed)
			{
				state = GameState::MainMenu;
			}

			renderer->Clear(Renderer::Color(0.04f, 0.05f, 0.09f, 1.0f));
			particles.Render(renderer);
			RenderControls();
			return;
		}

		// -------------------------------------------------------------
		// 4. PAUSE TOGGLE (ESC) & PAUSED STATE
		// -------------------------------------------------------------
		if (inputManager->GetRegisteredKeyState(Input::Esc) == Input::Pressed)
		{
			if (state == GameState::Playing)
			{
				state = GameState::Paused;
			}
			else if (state == GameState::Paused)
			{
				state = GameState::Playing;
			}
		}

		if (state == GameState::Paused)
		{
			if (inputManager->GetRegisteredKeyState(Input::R) == Input::Pressed)
			{
				RestartLevel();
			}
			if (inputManager->GetRegisteredKeyState(Input::M) == Input::Pressed)
			{
				state = GameState::MainMenu;
			}

			// In Pause menu: level jumping only allowed in Practice Mode!
			if (mode == GameMode::Practice)
			{
				if (inputManager->GetRegisteredKeyState(Input::One) == Input::Pressed) JumpToLevel(1);
				if (inputManager->GetRegisteredKeyState(Input::Two) == Input::Pressed) JumpToLevel(2);
				if (inputManager->GetRegisteredKeyState(Input::Three) == Input::Pressed) JumpToLevel(3);
				if (inputManager->GetRegisteredKeyState(Input::Four) == Input::Pressed) JumpToLevel(4);
				if (inputManager->GetRegisteredKeyState(Input::Five) == Input::Pressed) JumpToLevel(5);
			}

			// Render paused screen
			renderer->Clear(Renderer::Color(0.04f, 0.05f, 0.09f, 1.0f));
			level.Render(renderer, particles);
			player.Render(renderer);
			particles.Render(renderer);
			RenderHUD();
			RenderPause();
			return;
		}

		// -------------------------------------------------------------
		// 5. PLAYING STATE
		// -------------------------------------------------------------
		// Quick restart shortcut (R)
		if (inputManager->GetRegisteredKeyState(Input::R) == Input::Pressed)
		{
			RestartLevel();
		}

		// Quick level selection shortcuts (1 to 5) - ONLY IN PRACTICE MODE!
		// In a Full Run, level skipping is disabled so it is guaranteed to be a real run.
		if (mode == GameMode::Practice)
		{
			if (inputManager->GetRegisteredKeyState(Input::One) == Input::Pressed) JumpToLevel(1);
			if (inputManager->GetRegisteredKeyState(Input::Two) == Input::Pressed) JumpToLevel(2);
			if (inputManager->GetRegisteredKeyState(Input::Three) == Input::Pressed) JumpToLevel(3);
			if (inputManager->GetRegisteredKeyState(Input::Four) == Input::Pressed) JumpToLevel(4);
			if (inputManager->GetRegisteredKeyState(Input::Five) == Input::Pressed) JumpToLevel(5);
		}

		if (state == GameState::Playing)
		{
			totalTime += dt;
			levelTime += dt;

			level.Update(dt, particles);
			player.Update(dt, inputManager, level, particles);
			particles.Update(dt);

			if (player.HasDiedThisFrame())
			{
				totalDeaths++;
			}

			if (player.HasRespawnedThisFrame())
			{
				levelTime = 0.0f; // Reset attempt timer on respawn!
				level.Reset();    // Reset dash resets (crystals) and springs on respawn!
			}

			if (player.HasReachedGoal())
			{
				state = GameState::LevelClear;
				levelClearTimer = 2.4f;

				int curLvl = level.GetCurrentIndex();
				if (curLvl >= 1 && curLvl <= 5)
				{
					runLevelTimes[curLvl] = levelTime; // Save this playthrough's level time
					prevBestTime = bestLevelTimes[curLvl];
					if (prevBestTime <= 0.0f || levelTime < prevBestTime)
					{
						isNewGold = true;
						runIsGold[curLvl] = true;
						bestLevelTimes[curLvl] = levelTime;
						SaveRecords();
						particles.SpawnCrystalCollect(player.GetX() + 14.0f, player.GetY() + 14.0f);
					}
					else if (std::abs(levelTime - prevBestTime) < 0.005f)
					{
						isNewGold = true;
						runIsGold[curLvl] = true;
					}
					else
					{
						isNewGold = false;
					}
				}
			}
		}
		else if (state == GameState::LevelClear)
		{
			totalTime += dt;
			level.Update(dt, particles);
			particles.Update(dt);

			// Automatic countdown progression: player is forced to pause & review stats
			levelClearTimer -= dt;
			if (levelClearTimer <= 0.0f)
			{
				if (level.GetCurrentIndex() < level.GetTotalLevels())
				{
					NextLevel();
				}
				else
				{
					state = GameState::GameComplete;
				}
			}
		}
		else if (state == GameState::GameComplete)
		{
			particles.Update(dt);
			// Press Space, Enter, or R to restart full game; M to return to Main Menu
			if (inputManager->GetRegisteredKeyState(Input::Space) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::Enter) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::R) == Input::Pressed)
			{
				Start();
			}
			if (inputManager->GetRegisteredKeyState(Input::M) == Input::Pressed ||
			    inputManager->GetRegisteredKeyState(Input::Esc) == Input::Pressed)
			{
				state = GameState::MainMenu;
			}
		}

		// -------------------------------------------------------------
		// RENDERING
		// -------------------------------------------------------------
		// Deep slate blue space background
		renderer->Clear(Renderer::Color(0.05f, 0.07f, 0.12f, 1.0f));

		// Draw Level Tiles & Entities
		level.Render(renderer, particles);

		// Draw Player Cube
		player.Render(renderer);

		// Draw Particle Systems & Ghost Trails
		particles.Render(renderer);

		// Draw HUD Overlay
		RenderHUD();

		if (state == GameState::LevelClear)
		{
			RenderLevelClear();
		}
		else if (state == GameState::GameComplete)
		{
			RenderGameComplete();
		}
	}

	void Game::NextLevel()
	{
		int next = level.GetCurrentIndex() + 1;
		level.LoadLevel(next);
		player.Spawn(level.GetSpawnX(), level.GetSpawnY());
		particles.Clear();
		levelTime = 0.0f;
		state = GameState::Playing;
	}

	void Game::RestartLevel()
	{
		level.Reset();
		player.Spawn(level.GetSpawnX(), level.GetSpawnY());
		particles.Clear();
		levelTime = 0.0f;
		int cur = level.GetCurrentIndex();
		if (cur >= 1 && cur <= 5) runIsGold[cur] = false;
		state = GameState::Playing;
	}

	void Game::JumpToLevel(int index)
	{
		level.LoadLevel(index);
		player.Spawn(level.GetSpawnX(), level.GetSpawnY());
		particles.Clear();
		levelTime = 0.0f;
		if (index >= 1 && index <= 5) runIsGold[index] = false;
		state = GameState::Playing;
	}

	void Game::RenderMainMenu()
	{
		// Title Card Glow / Backdrop
		renderer->DrawFilledRectangle(340, 50, 940, 240, Renderer::Color(0.06f, 0.09f, 0.16f, 0.88f));
		renderer->DrawRectangle(340, 50, 940, 240, Renderer::Color(0.20f, 0.35f, 0.55f, 1.0f));
		renderer->DrawRectangle(342, 52, 938, 238, Renderer::Color(0.12f, 0.22f, 0.38f, 1.0f));

		// Title & Subtitle (Centered)
		DrawCenteredString(renderer, 75, "CUBE DASH", Renderer::Color(0.1f, 0.85f, 1.0f, 1.0f), 4.2f);
		DrawCenteredString(renderer, 138, "A CELESTE-INSPIRED PLATFORMER", Renderer::Color(0.75f, 0.85f, 0.98f, 1.0f), 1.45f);

		// Animated floating cube demonstration inside title box
		float floatY = std::sin(menuAnimTimer * 3.0f) * 6.0f;
		int cubeX = 624;
		int cubeY = static_cast<int>(180.0f + floatY);
		// Cube body
		renderer->DrawFilledRectangle(cubeX, cubeY, cubeX + 32, cubeY + 32, Renderer::Color(0.0f, 0.75f, 1.0f, 1.0f));
		renderer->DrawRectangle(cubeX, cubeY, cubeX + 32, cubeY + 32, Renderer::Color(0.6f, 0.95f, 1.0f, 1.0f));
		renderer->DrawRectangle(cubeX + 1, cubeY + 1, cubeX + 31, cubeY + 31, Renderer::Color(0.6f, 0.95f, 1.0f, 1.0f));

		// Menu Container Box
		int boxX1 = 440;
		int boxX2 = 840;
		int boxY1 = 275;
		int boxY2 = 565;
		renderer->DrawFilledRectangle(boxX1, boxY1, boxX2, boxY2, Renderer::Color(0.05f, 0.07f, 0.13f, 0.94f));
		renderer->DrawRectangle(boxX1, boxY1, boxX2, boxY2, Renderer::Color(0.22f, 0.32f, 0.48f, 1.0f));
		renderer->DrawRectangle(boxX1 + 1, boxY1 + 1, boxX2 - 1, boxY2 - 1, Renderer::Color(0.15f, 0.22f, 0.35f, 1.0f));

		const char* menuLabels[4] = {
			"FULL RUN",
			"PRACTICE",
			"CONTROLS",
			"QUIT"
		};

		for (int i = 0; i < 4; ++i)
		{
			int optY = 310 + i * 58;
			bool selected = (mainMenuIndex == i);

			if (selected)
			{
				// Highlight pill behind selected option
				renderer->DrawFilledRectangle(boxX1 + 20, optY - 8, boxX2 - 20, optY + 32, Renderer::Color(0.12f, 0.22f, 0.38f, 0.85f));
				renderer->DrawRectangle(boxX1 + 20, optY - 8, boxX2 - 20, optY + 32, Renderer::Color(0.2f, 0.9f, 1.0f, 1.0f));

				std::string selText = std::string(">  ") + menuLabels[i] + "  <";
				DrawCenteredStringInBox(renderer, boxX1, boxX2, optY, selText, Renderer::Color(0.2f, 0.9f, 1.0f, 1.0f), 1.9f);
			}
			else
			{
				DrawCenteredStringInBox(renderer, boxX1, boxX2, optY, menuLabels[i], Renderer::Color(0.65f, 0.72f, 0.85f, 1.0f), 1.7f);
			}
		}

		// Footer Navigation Hints
		DrawCenteredString(renderer, 610, "FULL RUN: Play 1-5 in sequence   |   PRACTICE: Pick level & switch with 1-5", Renderer::Color(0.6f, 0.75f, 0.9f, 0.9f), 1.25f);
		DrawCenteredString(renderer, 648, "W / S or UP / DOWN : Navigate   |   SPACE / ENTER : Select", Renderer::Color(0.40f, 0.50f, 0.65f, 0.85f), 1.15f);
	}

	void Game::RenderLevelSelect()
	{
		DrawCenteredString(renderer, 45, "PRACTICE - LEVEL SELECT", Renderer::Color(0.2f, 0.85f, 1.0f, 1.0f), 3.0f);
		DrawCenteredString(renderer, 90, "CHOOSE A LEVEL TO PRACTICE (SWITCH WITH 1-5 AT ANY TIME)", Renderer::Color(0.7f, 0.8f, 0.95f, 1.0f), 1.30f);

		int boxX1 = 260;
		int boxX2 = 1020;
		int boxY1 = 130;
		int boxY2 = 595;
		renderer->DrawFilledRectangle(boxX1, boxY1, boxX2, boxY2, Renderer::Color(0.04f, 0.06f, 0.11f, 0.95f));
		renderer->DrawRectangle(boxX1, boxY1, boxX2, boxY2, Renderer::Color(0.25f, 0.35f, 0.52f, 1.0f));

		const char* levelTitles[5] = {
			"LEVEL 1: FIRST STEPS",
			"LEVEL 2: WALL JUMPER",
			"LEVEL 3: DASH DANCE",
			"LEVEL 4: CRYSTAL CHASM",
			"LEVEL 5: SUMMIT PINNACLE"
		};

		for (int i = 1; i <= 5; ++i)
		{
			int optY = 150 + (i - 1) * 82;
			bool selected = (levelSelectIndex == i);

			if (selected)
			{
				renderer->DrawFilledRectangle(boxX1 + 24, optY - 6, boxX2 - 24, optY + 62, Renderer::Color(0.12f, 0.22f, 0.38f, 0.90f));
				renderer->DrawRectangle(boxX1 + 24, optY - 6, boxX2 - 24, optY + 62, Renderer::Color(0.2f, 0.9f, 1.0f, 1.0f));
				renderer->DrawRectangle(boxX1 + 25, optY - 5, boxX2 - 25, optY + 61, Renderer::Color(1.0f, 0.84f, 0.0f, 1.0f));

				std::string titleStr = std::string(">  ") + levelTitles[i - 1] + "  <";
				DrawCenteredStringInBox(renderer, boxX1, boxX2, optY + 4, titleStr, Renderer::Color(1.0f, 0.84f, 0.0f, 1.0f), 1.8f);
			}
			else
			{
				DrawCenteredStringInBox(renderer, boxX1, boxX2, optY + 4, levelTitles[i - 1], Renderer::Color(0.70f, 0.78f, 0.90f, 1.0f), 1.6f);
			}

			float best = bestLevelTimes[i];
			std::string recordStr = (best > 0.0f) ? ("BEST GOLD TIME: " + FormatTime(best)) : "BEST TIME: --:--.-- (NO RECORD)";
			Renderer::Color recColor = (best > 0.0f) ? Renderer::Color(1.0f, 0.84f, 0.0f, 1.0f) : Renderer::Color(0.45f, 0.52f, 0.65f, 0.8f);
			DrawCenteredStringInBox(renderer, boxX1, boxX2, optY + 34, recordStr, recColor, 1.25f);
		}

		DrawCenteredString(renderer, 630, "A / D or W / S : Select Level   |   SPACE / ENTER : Play   |   ESC / M : Main Menu", Renderer::Color(0.6f, 0.75f, 0.9f, 0.9f), 1.3f);
	}

	void Game::RenderControls()
	{
		DrawCenteredString(renderer, 40, "HOW TO PLAY & CONTROLS", Renderer::Color(0.15f, 0.85f, 1.0f, 1.0f), 3.0f);
		DrawCenteredString(renderer, 82, "MASTER THE CELESTE-STYLE PRECISION MOVEMENT", Renderer::Color(0.7f, 0.8f, 0.95f, 1.0f), 1.35f);

		int boxX1 = 180;
		int boxX2 = 1100;
		int boxY1 = 115;
		int boxY2 = 615;
		renderer->DrawFilledRectangle(boxX1, boxY1, boxX2, boxY2, Renderer::Color(0.04f, 0.06f, 0.11f, 0.96f));
		renderer->DrawRectangle(boxX1, boxY1, boxX2, boxY2, Renderer::Color(0.24f, 0.36f, 0.54f, 1.0f));

		// Section 1: Movement
		renderer->DrawString(boxX1 + 40, 135, "=== CORE MOVEMENT ===", Renderer::Color(1.0f, 0.84f, 0.0f, 1.0f), 1.5f);
		renderer->DrawString(boxX1 + 40, 165, "A / D or LEFT / RIGHT  : Move Left / Right (Snappy ground & air momentum)", Renderer::Color::White, 1.3f);
		renderer->DrawString(boxX1 + 40, 190, "W / S or UP / DOWN     : Aim Direction (Aim up, down, or diagonals)", Renderer::Color::White, 1.3f);
		renderer->DrawString(boxX1 + 40, 215, "SPACE / C / Z          : Jump (Hold key longer to jump higher! Coyote & Buffer enabled)", Renderer::Color::White, 1.3f);

		// Section 2: Dash & Wall Mechanics
		renderer->DrawString(boxX1 + 40, 250, "=== DASH & ADVANCED TECHNIQUES ===", Renderer::Color(0.2f, 0.9f, 1.0f, 1.0f), 1.5f);
		renderer->DrawString(boxX1 + 40, 278, "J / X / K / SHIFT      : 8-Directional Dash (Momentum boost with brief freeze-frame)", Renderer::Color::White, 1.3f);
		renderer->DrawString(boxX1 + 40, 301, "WAVEDASH / HYPER       : Dash down-diagonal into ground + JUMP (extreme speed & dash refill!)", Renderer::Color(0.3f, 1.0f, 0.6f, 1.0f), 1.25f);
		renderer->DrawString(boxX1 + 40, 324, "WALL SLIDE             : Hold movement towards a wall while falling to slide slowly", Renderer::Color::White, 1.3f);
		renderer->DrawString(boxX1 + 40, 347, "NEUTRAL WALL JUMP      : Release direction + Jump against wall to gain vertical height", Renderer::Color::White, 1.3f);
		renderer->DrawString(boxX1 + 40, 370, "AWAY WALL JUMP         : Hold away + Jump against wall to launch far horizontally", Renderer::Color::White, 1.3f);

		// Section 3: Interactive Entities & Shortcuts
		renderer->DrawString(boxX1 + 40, 400, "=== INTERACTIVE MECHANICS & SHORTCUTS ===", Renderer::Color(0.3f, 1.0f, 0.6f, 1.0f), 1.5f);
		renderer->DrawString(boxX1 + 40, 430, "GREEN CRYSTALS         : Replenishes your Dash mid-air instantly", Renderer::Color::White, 1.3f);
		renderer->DrawString(boxX1 + 40, 455, "SPRING PADS            : Launches your cube soaring high into the air", Renderer::Color::White, 1.3f);
		renderer->DrawString(boxX1 + 40, 480, "R                      : Quick restart current level", Renderer::Color::White, 1.3f);
		renderer->DrawString(boxX1 + 40, 505, "1 - 5                  : Instant jump to any level", Renderer::Color::White, 1.3f);
		renderer->DrawString(boxX1 + 40, 530, "ESC                    : Pause / Resume game", Renderer::Color::White, 1.3f);

		renderer->DrawLine(boxX1 + 30, 565, boxX2 - 30, 565, Renderer::Color(0.24f, 0.36f, 0.54f, 1.0f));
		DrawCenteredStringInBox(renderer, boxX1, boxX2, 578, "Press SPACE, ENTER, or ESC to Return to Main Menu", Renderer::Color(1.0f, 0.84f, 0.0f, 1.0f), 1.4f);
	}

	void Game::RenderHUD()
	{
		// Top HUD Bar background (y in [0, 40])
		renderer->DrawFilledRectangle(0, 0, 1280, 40, Renderer::Color(0.03f, 0.04f, 0.07f, 0.90f));
		renderer->DrawLine(0, 40, 1280, 40, Renderer::Color(0.18f, 0.25f, 0.38f, 1.0f));

		// Level Title
		renderer->DrawString(24, 13, level.GetName(), Renderer::Color::White, 1.35f);

		// Mode Indicator Badge: Clearly distinguishes real full runs from practice!
		if (mode == GameMode::FullRun)
		{
			renderer->DrawFilledRectangle(252, 9, 342, 31, Renderer::Color(0.08f, 0.22f, 0.38f, 0.90f));
			renderer->DrawRectangle(252, 9, 342, 31, Renderer::Color(0.2f, 0.85f, 1.0f, 0.85f));
			renderer->DrawString(259, 14, "FULL RUN", Renderer::Color(0.3f, 0.95f, 1.0f, 1.0f), 1.1f);
		}
		else
		{
			renderer->DrawFilledRectangle(252, 9, 342, 31, Renderer::Color(0.28f, 0.18f, 0.08f, 0.90f));
			renderer->DrawRectangle(252, 9, 342, 31, Renderer::Color(1.0f, 0.70f, 0.2f, 0.85f));
			renderer->DrawString(259, 14, "PRACTICE", Renderer::Color(1.0f, 0.82f, 0.25f, 1.0f), 1.1f);
		}

		// Dash Indicator Icon (Cyan diamond when ready, dim ruby when empty)
		int dashIconX = 380;
		int dashIconY = 20;
		bool dashReady = (player.GetDashesAvailable() > 0);
		Renderer::Color dashColor = dashReady ? Renderer::Color(0.2f, 0.9f, 1.0f, 1.0f) : Renderer::Color(0.5f, 0.2f, 0.25f, 0.6f);

		renderer->DrawLine(dashIconX, dashIconY - 8, dashIconX + 8, dashIconY, dashColor);
		renderer->DrawLine(dashIconX + 8, dashIconY, dashIconX, dashIconY + 8, dashColor);
		renderer->DrawLine(dashIconX, dashIconY + 8, dashIconX - 8, dashIconY, dashColor);
		renderer->DrawLine(dashIconX - 8, dashIconY, dashIconX, dashIconY - 8, dashColor);
		if (dashReady)
		{
			renderer->DrawFilledRectangle(dashIconX - 3, dashIconY - 3, dashIconX + 3, dashIconY + 3, Renderer::Color::White);
		}
		renderer->DrawString(dashIconX + 16, 13, dashReady ? "DASH READY" : "DASH EMPTY", dashColor, 1.25f);

		// Current Level Attempt Time
		int curLvl = level.GetCurrentIndex();
		float best = (curLvl >= 1 && curLvl <= 5) ? bestLevelTimes[curLvl] : 0.0f;
		std::string timeStr = "TIME: " + FormatTime(levelTime);
		renderer->DrawString(540, 13, timeStr, Renderer::Color(0.95f, 0.95f, 0.95f, 1.0f), 1.35f);

		// Fastest Level (Gold) Time
		std::string goldStr = (best > 0.0f) ? ("BEST: " + FormatTime(best)) : "BEST: --:--.--";
		renderer->DrawString(730, 13, goldStr, Renderer::Color(1.0f, 0.84f, 0.0f, 1.0f), 1.35f);

		// Total Speedrun Timer
		std::string totalStr = "TOTAL: " + FormatTime(totalTime);
		renderer->DrawString(930, 13, totalStr, Renderer::Color(0.75f, 0.85f, 1.0f, 0.9f), 1.3f);

		// Death Counter
		std::string deathStr = "DEATHS: " + std::to_string(totalDeaths);
		renderer->DrawString(1130, 13, deathStr, Renderer::Color(1.0f, 0.4f, 0.45f, 1.0f), 1.3f);

		// Bottom subtle control bar (y in [680, 720])
		renderer->DrawFilledRectangle(0, 680, 1280, 720, Renderer::Color(0.02f, 0.03f, 0.06f, 0.92f));
		renderer->DrawLine(0, 680, 1280, 680, Renderer::Color(0.14f, 0.18f, 0.28f, 1.0f));
		renderer->DrawString(24, 694, level.GetHint(), Renderer::Color(0.6f, 0.75f, 0.9f, 0.85f), 1.2f);
		if (mode == GameMode::Practice)
		{
			renderer->DrawString(840, 694, "R: Restart  |  1-5: Switch Level  |  ESC: Pause", Renderer::Color(0.5f, 0.6f, 0.7f, 0.75f), 1.15f);
		}
		else
		{
			renderer->DrawString(920, 694, "R: Restart Level  |  ESC: Pause", Renderer::Color(0.5f, 0.6f, 0.7f, 0.75f), 1.15f);
		}
	}

	void Game::RenderLevelClear()
	{
		// Dim backdrop
		renderer->DrawFilledRectangle(0, 0, 1280, 720, Renderer::Color(0.0f, 0.0f, 0.0f, 0.50f));

		// Centered Dialog Card: 600px wide (x: 340 to 940), 320px tall (y: 200 to 520)
		int cardX1 = 340;
		int cardX2 = 940;
		int cardY1 = 200;
		int cardY2 = 520;

		renderer->DrawFilledRectangle(cardX1, cardY1, cardX2, cardY2, Renderer::Color(0.03f, 0.05f, 0.10f, 0.96f));
		Renderer::Color borderColor = Renderer::Color(0.24f, 0.55f, 0.85f, 1.0f);
		renderer->DrawRectangle(cardX1, cardY1, cardX2, cardY2, borderColor);
		renderer->DrawRectangle(cardX1 + 1, cardY1 + 1, cardX2 - 1, cardY2 - 1, borderColor);
		renderer->DrawRectangle(cardX1 + 3, cardY1 + 3, cardX2 - 3, cardY2 - 3, Renderer::Color(0.12f, 0.22f, 0.38f, 0.8f));

		int curLvl = level.GetCurrentIndex();
		float best = (curLvl >= 1 && curLvl <= 5) ? bestLevelTimes[curLvl] : levelTime;

		std::ostringstream ss;
		ss << std::fixed << std::setprecision(1) << std::max(0.0f, levelClearTimer) << "s";

		if (isNewGold)
		{
			DrawCenteredStringInBox(renderer, cardX1, cardX2, 230, "* NEW GOLD TIME! *", Renderer::Color(1.0f, 0.84f, 0.0f, 1.0f), 2.5f);
			DrawCenteredStringInBox(renderer, cardX1, cardX2, 275, "LEVEL CLEAR!", Renderer::Color::White, 1.8f);

			DrawCenteredStringInBox(renderer, cardX1, cardX2, 325, "Time: " + FormatTime(levelTime), Renderer::Color(1.0f, 0.84f, 0.0f, 1.0f), 2.2f);

			if (prevBestTime > 0.0f)
			{
				DrawCenteredStringInBox(renderer, cardX1, cardX2, 375, "Previous Best: " + FormatTime(prevBestTime), Renderer::Color(0.8f, 0.8f, 0.85f, 1.0f), 1.4f);
			}
			else
			{
				DrawCenteredStringInBox(renderer, cardX1, cardX2, 375, "FIRST RECORD SET!", Renderer::Color(0.8f, 0.8f, 0.85f, 1.0f), 1.4f);
			}

			renderer->DrawLine(cardX1 + 60, 420, cardX2 - 60, 420, Renderer::Color(0.20f, 0.35f, 0.55f, 0.8f));
			DrawCenteredStringInBox(renderer, cardX1, cardX2, 445, "Preparing next level...", Renderer::Color(0.7f, 0.82f, 0.95f, 0.9f), 1.4f);
			DrawCenteredStringInBox(renderer, cardX1, cardX2, 478, "(Advancing automatically in " + ss.str() + ")", Renderer::Color(0.45f, 0.55f, 0.70f, 0.8f), 1.15f);
		}
		else
		{
			DrawCenteredStringInBox(renderer, cardX1, cardX2, 245, "LEVEL CLEAR!", Renderer::Color(0.3f, 1.0f, 0.6f, 1.0f), 2.8f);
			DrawCenteredStringInBox(renderer, cardX1, cardX2, 310, "Time: " + FormatTime(levelTime), Renderer::Color::White, 2.2f);
			DrawCenteredStringInBox(renderer, cardX1, cardX2, 360, "Best: " + FormatTime(best), Renderer::Color(1.0f, 0.84f, 0.0f, 1.0f), 1.6f);

			renderer->DrawLine(cardX1 + 60, 415, cardX2 - 60, 415, Renderer::Color(0.20f, 0.35f, 0.55f, 0.8f));
			DrawCenteredStringInBox(renderer, cardX1, cardX2, 445, "Preparing next level...", Renderer::Color(0.7f, 0.82f, 0.95f, 0.9f), 1.4f);
			DrawCenteredStringInBox(renderer, cardX1, cardX2, 478, "(Advancing automatically in " + ss.str() + ")", Renderer::Color(0.45f, 0.55f, 0.70f, 0.8f), 1.15f);
		}
	}

	void Game::RenderGameComplete()
	{
		// Backdrop
		renderer->DrawFilledRectangle(0, 0, 1280, 720, Renderer::Color(0.0f, 0.0f, 0.0f, 0.60f));

		// Victory Screen Dialog: 760px wide (x: 260 to 1020), 570px tall (y: 75 to 645)
		int boxX1 = 260;
		int boxX2 = 1020;
		int boxY1 = 75;
		int boxY2 = 645;

		renderer->DrawFilledRectangle(boxX1, boxY1, boxX2, boxY2, Renderer::Color(0.03f, 0.05f, 0.10f, 0.97f));
		// Clean Cyan / Slate Blue borders (NO harsh yellow lines!)
		renderer->DrawRectangle(boxX1, boxY1, boxX2, boxY2, Renderer::Color(0.24f, 0.55f, 0.85f, 1.0f));
		renderer->DrawRectangle(boxX1 + 1, boxY1 + 1, boxX2 - 1, boxY2 - 1, Renderer::Color(0.14f, 0.35f, 0.60f, 1.0f));

		if (mode == GameMode::FullRun)
		{
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 100, "CONGRATULATIONS!", Renderer::Color(0.2f, 0.9f, 1.0f, 1.0f), 3.0f);
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 145, "OFFICIAL FULL RUN COMPLETED!", Renderer::Color::White, 1.8f);
		}
		else
		{
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 100, "PRACTICE COMPLETED!", Renderer::Color(1.0f, 0.80f, 0.25f, 1.0f), 3.0f);
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 145, "ALL LEVELS CLEARED IN PRACTICE MODE", Renderer::Color::White, 1.8f);
		}

		renderer->DrawLine(boxX1 + 50, 180, boxX2 - 50, 180, Renderer::Color(0.22f, 0.36f, 0.55f, 1.0f));

		DrawCenteredStringInBox(renderer, boxX1, boxX2, 198, "OVERALL RUN RESULTS", Renderer::Color(0.2f, 0.85f, 1.0f, 1.0f), 1.8f);
		DrawCenteredStringInBox(renderer, boxX1, boxX2, 235, "Total Run Time:  " + FormatTime(totalTime), Renderer::Color::White, 1.7f);
		DrawCenteredStringInBox(renderer, boxX1, boxX2, 268, "Total Deaths:    " + std::to_string(totalDeaths), Renderer::Color(1.0f, 0.45f, 0.45f, 1.0f), 1.7f);

		renderer->DrawLine(boxX1 + 50, 305, boxX2 - 50, 305, Renderer::Color(0.22f, 0.36f, 0.55f, 1.0f));

		// Show this run's level times (Gold times highlighted in gold with [GOLD] badge!)
		DrawCenteredStringInBox(renderer, boxX1, boxX2, 322, "THIS RUN'S LEVEL TIMES:", Renderer::Color(0.2f, 0.9f, 1.0f, 1.0f), 1.6f);
		for (int i = 1; i <= 5; ++i)
		{
			int yPos = 358 + (i - 1) * 32;
			std::string lvlStr = "Level " + std::to_string(i) + ":";
			std::string timeStr = (runLevelTimes[i] > 0.0f) ? FormatTime(runLevelTimes[i]) : "--:--.--";

			bool isGold = (runLevelTimes[i] > 0.0f) && (runIsGold[i] || (bestLevelTimes[i] > 0.0f && runLevelTimes[i] <= bestLevelTimes[i] + 0.005f));

			if (isGold)
			{
				renderer->DrawString(430, yPos, lvlStr, Renderer::Color(1.0f, 0.90f, 0.40f, 1.0f), 1.5f);
				renderer->DrawString(610, yPos, timeStr, Renderer::Color(1.0f, 0.84f, 0.0f, 1.0f), 1.5f);
				renderer->DrawString(765, yPos, "[GOLD]", Renderer::Color(1.0f, 0.84f, 0.0f, 1.0f), 1.4f);
			}
			else
			{
				renderer->DrawString(430, yPos, lvlStr, Renderer::Color::White, 1.5f);
				renderer->DrawString(610, yPos, timeStr, Renderer::Color(0.85f, 0.92f, 1.0f, 1.0f), 1.5f);
			}
		}

		renderer->DrawLine(boxX1 + 50, 535, boxX2 - 50, 535, Renderer::Color(0.22f, 0.36f, 0.55f, 1.0f));
		DrawCenteredStringInBox(renderer, boxX1, boxX2, 555, "Press SPACE, ENTER, or R to Play Again!", Renderer::Color(0.3f, 1.0f, 0.6f, 1.0f), 1.7f);
		DrawCenteredStringInBox(renderer, boxX1, boxX2, 595, "Press M to Return to Main Menu", Renderer::Color(0.7f, 0.8f, 0.9f, 0.85f), 1.35f);
	}

	void Game::RenderPause()
	{
		// Backdrop
		renderer->DrawFilledRectangle(0, 0, 1280, 720, Renderer::Color(0.0f, 0.0f, 0.0f, 0.45f));

		// Centered Dialog: 440px wide (x: 420 to 860), 320px tall (y: 200 to 520)
		int boxX1 = 420;
		int boxX2 = 860;
		int boxY1 = 200;
		int boxY2 = 520;

		renderer->DrawFilledRectangle(boxX1, boxY1, boxX2, boxY2, Renderer::Color(0.04f, 0.06f, 0.12f, 0.96f));
		renderer->DrawRectangle(boxX1, boxY1, boxX2, boxY2, Renderer::Color(0.3f, 0.6f, 0.9f, 1.0f));
		renderer->DrawRectangle(boxX1 + 1, boxY1 + 1, boxX2 - 1, boxY2 - 1, Renderer::Color(0.18f, 0.28f, 0.45f, 1.0f));

		DrawCenteredStringInBox(renderer, boxX1, boxX2, 225, "PAUSED", Renderer::Color::White, 2.6f);
		renderer->DrawLine(boxX1 + 40, 268, boxX2 - 40, 268, Renderer::Color(0.25f, 0.38f, 0.55f, 1.0f));

		if (mode == GameMode::Practice)
		{
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 290, "[ ESC ] Resume Practice", Renderer::Color(1.0f, 0.85f, 0.2f, 1.0f), 1.55f);
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 330, "[ R ] Restart Level", Renderer::Color(0.3f, 0.9f, 1.0f, 1.0f), 1.55f);
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 370, "[ 1 - 5 ] Switch Level", Renderer::Color(0.85f, 0.90f, 0.95f, 1.0f), 1.45f);
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 410, "[ M ] Return to Main Menu", Renderer::Color(1.0f, 0.45f, 0.45f, 1.0f), 1.55f);
		}
		else
		{
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 295, "[ ESC ] Resume Full Run", Renderer::Color(1.0f, 0.85f, 0.2f, 1.0f), 1.55f);
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 340, "[ R ] Restart Level", Renderer::Color(0.3f, 0.9f, 1.0f, 1.0f), 1.55f);
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 385, "[ M ] Return to Main Menu", Renderer::Color(1.0f, 0.45f, 0.45f, 1.0f), 1.55f);
			DrawCenteredStringInBox(renderer, boxX1, boxX2, 428, "(Level skipping disabled in Full Run)", Renderer::Color(0.5f, 0.6f, 0.72f, 0.8f), 1.15f);
		}

		renderer->DrawLine(boxX1 + 40, 460, boxX2 - 40, 460, Renderer::Color(0.20f, 0.30f, 0.45f, 1.0f));
		DrawCenteredStringInBox(renderer, boxX1, boxX2, 478, "Best records automatically saved", Renderer::Color(0.5f, 0.6f, 0.75f, 0.8f), 1.15f);
	}

	void Game::End()
	{
	}
}
