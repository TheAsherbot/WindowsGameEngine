#include "Level.h"
#include <cmath>
#include <algorithm>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace Games
{
	Level::Level()
	{
		LoadLevel(1);
	}

	void Level::ClearGrid()
	{
		for (int x = 0; x < GridWidth; x++)
		{
			for (int y = 0; y < GridHeight; y++)
			{
				tiles[x][y] = TileType::Empty;
			}
		}
		dashRefills.clear();
		springs.clear();
	}

	void Level::LoadLevel(int levelIndex)
	{
		currentLevelIndex = levelIndex;
		ClearGrid();

		switch (levelIndex)
		{
		case 1:
			BuildLevel1();
			break;
		case 2:
			BuildLevel2();
			break;
		case 3:
			BuildLevel3();
			break;
		case 4:
			BuildLevel4();
			break;
		case 5:
			BuildLevel5();
			break;
		default:
			BuildLevel1();
			break;
		}
	}

	void Level::Reset()
	{
		// Reactivate all dash refills
		for (auto& refill : dashRefills)
		{
			refill.active = true;
			refill.respawnTimer = 0.0f;
		}
	}

	void Level::Update(float dt, ParticleSystem& particles)
	{
		goalAnim += dt * 3.0f;

		// Update dash refill entities
		for (auto& refill : dashRefills)
		{
			refill.animTimer += dt * 3.5f;
			if (!refill.active)
			{
				refill.respawnTimer -= dt;
				if (refill.respawnTimer <= 0.0f)
				{
					refill.active = true;
					particles.SpawnCrystalCollect(refill.x + 16.0f, refill.y + 16.0f);
				}
			}
		}

		// Update springs
		for (auto& spring : springs)
		{
			if (spring.bounceTimer > 0.0f)
			{
				spring.bounceTimer -= dt * 6.0f;
				if (spring.bounceTimer < 0.0f) spring.bounceTimer = 0.0f;
			}
		}

		// Ambient particles at the goal portal
		particles.SpawnPortalSparkles(goalX + 16.0f, goalY + 16.0f, 24.0f);
	}

	void Level::Render(Renderer::DX12Renderer* renderer, ParticleSystem& particles)
	{
		// Render solid tiles & hazards
		for (int x = 0; x < GridWidth; x++)
		{
			for (int y = 0; y < GridHeight; y++)
			{
				TileType t = tiles[x][y];
				int px = x * TileSize;
				int py = TileOffsetY + y * TileSize;

				if (t == TileType::Solid)
				{
					// Solid ground with clean small border on everything
					renderer->DrawFilledRectangle(px, py, px + TileSize, py + TileSize, Renderer::Color(0.14f, 0.17f, 0.23f, 1.0f));
					renderer->DrawRectangle(px, py, px + TileSize, py + TileSize, Renderer::Color(0.30f, 0.38f, 0.50f, 1.0f));
				}
				else if (t == TileType::Platform)
				{
					// One-way semi-solid with border
					renderer->DrawFilledRectangle(px, py, px + TileSize, py + 8, Renderer::Color(0.20f, 0.28f, 0.40f, 1.0f));
					renderer->DrawRectangle(px, py, px + TileSize, py + 8, Renderer::Color(0.42f, 0.60f, 0.85f, 1.0f));
				}
				else if (t == TileType::SpikeUp)
				{
					// Clean solid hazard teeth with dark ruby border - ZERO yellow lines!
					Renderer::Color spikeFill(0.95f, 0.22f, 0.20f, 1.0f);
					Renderer::Color spikeEdge(0.45f, 0.08f, 0.10f, 1.0f);
					Renderer::Color baseFill(0.20f, 0.12f, 0.15f, 1.0f);
					Renderer::Color baseBorder(0.38f, 0.18f, 0.22f, 1.0f);

					renderer->DrawFilledRectangle(px, py + 26, px + TileSize, py + TileSize, baseFill);
					renderer->DrawRectangle(px, py + 26, px + TileSize, py + TileSize, baseBorder);

					int apices[2] = { px + 8, px + 24 };
					for (int ax : apices)
					{
						for (int sy = py + 4; sy < py + 26; sy++)
						{
							int w = ((sy - (py + 4)) * 7) / 22;
							renderer->DrawFilledRectangle(ax - w, sy, ax + w + 1, sy + 1, spikeEdge);
							if (w >= 1)
							{
								renderer->DrawFilledRectangle(ax - w + 1, sy, ax + w, sy + 1, spikeFill);
							}
						}
					}
				}
				else if (t == TileType::SpikeDown)
				{
					Renderer::Color spikeFill(0.95f, 0.22f, 0.20f, 1.0f);
					Renderer::Color spikeEdge(0.45f, 0.08f, 0.10f, 1.0f);
					Renderer::Color baseFill(0.20f, 0.12f, 0.15f, 1.0f);
					Renderer::Color baseBorder(0.38f, 0.18f, 0.22f, 1.0f);

					renderer->DrawFilledRectangle(px, py, px + TileSize, py + 6, baseFill);
					renderer->DrawRectangle(px, py, px + TileSize, py + 6, baseBorder);

					int apices[2] = { px + 8, px + 24 };
					for (int ax : apices)
					{
						for (int sy = py + 6; sy < py + 28; sy++)
						{
							int w = (((py + 28) - sy) * 7) / 22;
							renderer->DrawFilledRectangle(ax - w, sy, ax + w + 1, sy + 1, spikeEdge);
							if (w >= 1)
							{
								renderer->DrawFilledRectangle(ax - w + 1, sy, ax + w, sy + 1, spikeFill);
							}
						}
					}
				}
				else if (t == TileType::SpikeLeft)
				{
					Renderer::Color spikeFill(0.95f, 0.22f, 0.20f, 1.0f);
					Renderer::Color spikeEdge(0.45f, 0.08f, 0.10f, 1.0f);
					Renderer::Color baseFill(0.20f, 0.12f, 0.15f, 1.0f);
					Renderer::Color baseBorder(0.38f, 0.18f, 0.22f, 1.0f);

					renderer->DrawFilledRectangle(px + 26, py, px + TileSize, py + TileSize, baseFill);
					renderer->DrawRectangle(px + 26, py, px + TileSize, py + TileSize, baseBorder);

					int apices[2] = { py + 8, py + 24 };
					for (int ay : apices)
					{
						for (int sx = px + 4; sx < px + 26; sx++)
						{
							int h = ((sx - (px + 4)) * 7) / 22;
							renderer->DrawFilledRectangle(sx, ay - h, sx + 1, ay + h + 1, spikeEdge);
							if (h >= 1)
							{
								renderer->DrawFilledRectangle(sx, ay - h + 1, sx + 1, ay + h, spikeFill);
							}
						}
					}
				}
				else if (t == TileType::SpikeRight)
				{
					Renderer::Color spikeFill(0.95f, 0.22f, 0.20f, 1.0f);
					Renderer::Color spikeEdge(0.45f, 0.08f, 0.10f, 1.0f);
					Renderer::Color baseFill(0.20f, 0.12f, 0.15f, 1.0f);
					Renderer::Color baseBorder(0.38f, 0.18f, 0.22f, 1.0f);

					renderer->DrawFilledRectangle(px, py, px + 6, py + TileSize, baseFill);
					renderer->DrawRectangle(px, py, px + 6, py + TileSize, baseBorder);

					int apices[2] = { py + 8, py + 24 };
					for (int ay : apices)
					{
						for (int sx = px + 6; sx < px + 28; sx++)
						{
							int h = (((px + 28) - sx) * 7) / 22;
							renderer->DrawFilledRectangle(sx, ay - h, sx + 1, ay + h + 1, spikeEdge);
							if (h >= 1)
							{
								renderer->DrawFilledRectangle(sx, ay - h + 1, sx + 1, ay + h, spikeFill);
							}
						}
					}
				}
			}
		}

		// Render Dash Refill Crystals
		for (const auto& refill : dashRefills)
		{
			float floatOffset = std::sin(refill.animTimer) * 4.0f;
			int cx = static_cast<int>(refill.x + 16.0f);
			int cy = static_cast<int>(refill.y + 16.0f + floatOffset);

			if (refill.active)
			{
				// Floating neon green crystal diamond (clean scanline geometry, NO vertical line artifacts)
				Renderer::Color cGreen(0.15f, 0.95f, 0.50f, 1.0f);
				Renderer::Color cCore(0.85f, 1.0f, 0.92f, 1.0f);
				for (int d = -9; d <= 9; ++d)
				{
					int w = 9 - std::abs(d);
					renderer->DrawFilledRectangle(cx - w, cy + d, cx + w + 1, cy + d + 1, cGreen);
				}
				for (int d = -4; d <= 4; ++d)
				{
					int w = 4 - std::abs(d);
					renderer->DrawFilledRectangle(cx - w, cy + d, cx + w + 1, cy + d + 1, cCore);
				}
			}
			else
			{
				// Dim ghost diamond while respawning
				Renderer::Color cDim(0.18f, 0.32f, 0.25f, 0.40f);
				for (int d = -6; d <= 6; ++d)
				{
					int w = 6 - std::abs(d);
					renderer->DrawFilledRectangle(cx - w, cy + d, cx + w + 1, cy + d + 1, cDim);
				}
			}
		}

		// Render Springs (Solid Celeste-style mechanical bounce pad - NO wireframe yellow lines)
		for (const auto& spring : springs)
		{
			int sx = static_cast<int>(spring.x);
			int sy = static_cast<int>(spring.y);
			float compress = spring.bounceTimer * 8.0f;

			// Base plate: sturdy metallic foundation with border
			renderer->DrawFilledRectangle(sx + 2, sy + 25, sx + 30, sy + 32, Renderer::Color(0.16f, 0.20f, 0.28f, 1.0f));
			renderer->DrawRectangle(sx + 2, sy + 25, sx + 30, sy + 32, Renderer::Color(0.32f, 0.42f, 0.58f, 1.0f));

			// Mechanical piston stem: solid central column
			int stemTop = static_cast<int>(sy + 14 + compress);
			renderer->DrawFilledRectangle(sx + 12, stemTop, sx + 20, sy + 25, Renderer::Color(0.55f, 0.65f, 0.78f, 1.0f));
			renderer->DrawRectangle(sx + 12, stemTop, sx + 20, sy + 25, Renderer::Color(0.28f, 0.36f, 0.48f, 1.0f));

			// Top bouncy launch pad: solid crimson-orange cushion with crisp border and highlight strip
			int padTop = static_cast<int>(sy + 8 + compress);
			int padBottom = static_cast<int>(sy + 15 + compress);
			renderer->DrawFilledRectangle(sx + 3, padTop, sx + 29, padBottom, Renderer::Color(0.95f, 0.30f, 0.20f, 1.0f));
			renderer->DrawRectangle(sx + 3, padTop, sx + 29, padBottom, Renderer::Color(1.0f, 0.70f, 0.55f, 1.0f));
			renderer->DrawLine(sx + 5, padTop + 2, sx + 27, padTop + 2, Renderer::Color(1.0f, 0.92f, 0.88f, 1.0f));
		}

		// Render Goal Portal
		{
			int gx = static_cast<int>(goalX + 16.0f);
			int gy = static_cast<int>(goalY + 16.0f);
			float pulse = std::sin(goalAnim) * 3.0f;

			// Outer rotating square / ring
			int r1 = static_cast<int>(18.0f + pulse);
			renderer->DrawRectangle(gx - r1, gy - r1, gx + r1, gy + r1, Renderer::Color(0.3f, 0.8f, 1.0f, 0.7f));

			// Middle ring
			int r2 = static_cast<int>(12.0f - pulse * 0.5f);
			renderer->DrawRectangle(gx - r2, gy - r2, gx + r2, gy + r2, Renderer::Color(0.9f, 0.4f, 1.0f, 0.8f));

			// Glowing white-gold core
			renderer->DrawFilledRectangle(gx - 6, gy - 6, gx + 6, gy + 6, Renderer::Color(1.0f, 1.0f, 0.9f, 0.95f));
		}
	}

	bool Level::IsSolidAt(int gridX, int gridY) const
	{
		if (gridX < 0 || gridX >= GridWidth || gridY < 0 || gridY >= GridHeight)
		{
			return true; // Screen boundaries are solid
		}
		return tiles[gridX][gridY] == TileType::Solid;
	}

	bool Level::CheckSolidOverlap(const AABB& box) const
	{
		int minX = std::max(0, static_cast<int>(box.Left()) / TileSize);
		int maxX = std::min(GridWidth - 1, static_cast<int>(box.Right()) / TileSize);
		int minY = std::max(0, (static_cast<int>(box.Top()) - TileOffsetY) / TileSize);
		int maxY = std::min(GridHeight - 1, (static_cast<int>(box.Bottom()) - TileOffsetY) / TileSize);

		// Boundary checks
		if (box.Left() < 0.0f || box.Right() > GridWidth * TileSize || box.Top() < static_cast<float>(TileOffsetY))
		{
			return true;
		}

		for (int x = minX; x <= maxX; x++)
		{
			for (int y = minY; y <= maxY; y++)
			{
				if (tiles[x][y] == TileType::Solid)
				{
					AABB tileBox = { static_cast<float>(x * TileSize), static_cast<float>(TileOffsetY + y * TileSize), static_cast<float>(TileSize), static_cast<float>(TileSize) };
					if (box.Overlaps(tileBox))
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	bool Level::CheckPlatformOverlap(const AABB& box, float prevBottom) const
	{
		int minX = std::max(0, static_cast<int>(box.Left()) / TileSize);
		int maxX = std::min(GridWidth - 1, static_cast<int>(box.Right()) / TileSize);
		int minY = std::max(0, (static_cast<int>(box.Top()) - TileOffsetY) / TileSize);
		int maxY = std::min(GridHeight - 1, (static_cast<int>(box.Bottom()) - TileOffsetY) / TileSize);

		for (int x = minX; x <= maxX; x++)
		{
			for (int y = minY; y <= maxY; y++)
			{
				if (tiles[x][y] == TileType::Platform)
				{
					float platformTop = static_cast<float>(TileOffsetY + y * TileSize);
					// One-way: only solid if player was above it previously and is moving down
					if (prevBottom <= platformTop + 2.0f && box.Bottom() >= platformTop)
					{
						AABB platBox = { static_cast<float>(x * TileSize), platformTop, static_cast<float>(TileSize), 6.0f };
						if (box.Overlaps(platBox))
						{
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	bool Level::CheckHazardOverlap(const AABB& box) const
	{
		// Falling below screen bottom is deadly
		if (box.Bottom() > TileOffsetY + GridHeight * TileSize + 16.0f)
		{
			return true;
		}

		int minX = std::max(0, static_cast<int>(box.Left()) / TileSize);
		int maxX = std::min(GridWidth - 1, static_cast<int>(box.Right()) / TileSize);
		int minY = std::max(0, (static_cast<int>(box.Top()) - TileOffsetY) / TileSize);
		int maxY = std::min(GridHeight - 1, (static_cast<int>(box.Bottom()) - TileOffsetY) / TileSize);

		for (int x = minX; x <= maxX; x++)
		{
			for (int y = minY; y <= maxY; y++)
			{
				TileType t = tiles[x][y];
				if (t >= TileType::SpikeUp && t <= TileType::SpikeRight)
				{
					// Slight forgiveness: spike hitbox is inset by 4 pixels!
					float tx = static_cast<float>(x * TileSize);
					float ty = static_cast<float>(TileOffsetY + y * TileSize);
					AABB spikeBox;

					if (t == TileType::SpikeUp)
						spikeBox = { tx + 4.0f, ty + 8.0f, static_cast<float>(TileSize - 8), static_cast<float>(TileSize - 8) };
					else if (t == TileType::SpikeDown)
						spikeBox = { tx + 4.0f, ty, static_cast<float>(TileSize - 8), static_cast<float>(TileSize - 8) };
					else if (t == TileType::SpikeLeft)
						spikeBox = { tx + 8.0f, ty + 4.0f, static_cast<float>(TileSize - 8), static_cast<float>(TileSize - 8) };
					else // SpikeRight
						spikeBox = { tx, ty + 4.0f, static_cast<float>(TileSize - 8), static_cast<float>(TileSize - 8) };

					if (box.Overlaps(spikeBox))
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	bool Level::CheckDashRefillOverlap(const AABB& box, ParticleSystem& particles)
	{
		for (auto& refill : dashRefills)
		{
			if (refill.active)
			{
				// Generous full-tile hitbox so the player never misses
				AABB refillBox = { refill.x - 2.0f, refill.y - 2.0f, static_cast<float>(TileSize + 4), static_cast<float>(TileSize + 4) };
				if (box.Overlaps(refillBox))
				{
					refill.active = false;
					refill.respawnTimer = 2.5f; // Respawns after 2.5s
					particles.SpawnCrystalCollect(refill.x + 16.0f, refill.y + 16.0f);
					return true;
				}
			}
		}
		return false;
	}

	bool Level::CheckSpringOverlap(const AABB& box, float& outBounceVy)
	{
		for (auto& spring : springs)
		{
			AABB springBox = { spring.x + 2.0f, spring.y + 6.0f, 28.0f, 26.0f };
			if (box.Overlaps(springBox))
			{
				spring.bounceTimer = 1.0f;
				outBounceVy = -1150.0f; // High soaring Celeste Super Bounce!
				return true;
			}
		}
		return false;
	}

	bool Level::CheckGoalOverlap(const AABB& box) const
	{
		AABB goalBox = { goalX + 4.0f, goalY + 4.0f, 24.0f, 24.0f };
		return box.Overlaps(goalBox);
	}

	// -------------------------------------------------------------
	// LEVEL 1: First Steps
	// Introduces horizontal run, jump, variable jump height & small gaps
	// -------------------------------------------------------------
	void Level::BuildLevel1()
	{
		levelName = "LEVEL 1: FIRST STEPS";
		levelHint = "A / D to Move | SPACE to Jump (hold longer to jump higher!)";
		spawnX = 64.0f;
		spawnY = static_cast<float>(TileOffsetY + 18 * TileSize - 28);
		goalX = 1184.0f;
		goalY = static_cast<float>(TileOffsetY + 8 * TileSize - 28);

		// Boundaries (Row 0 is ceiling, Row 19 is floor)
		for (int x = 0; x < GridWidth; x++)
		{
			tiles[x][0] = TileType::Solid; // Ceiling
			tiles[x][GridHeight - 1] = TileType::Solid; // Floor

			if (x == 12 || x == 13 || x == 24 || x == 25)
			{
				tiles[x][GridHeight - 1] = TileType::SpikeUp; // Hazard pits
			}
		}

		for (int y = 0; y < GridHeight; y++)
		{
			tiles[0][y] = TileType::Solid;
			tiles[GridWidth - 1][y] = TileType::Solid;
		}

		// Obstacles / Step platforms
		for (int x = 7; x <= 10; x++) tiles[x][16] = TileType::Solid;
		for (int x = 16; x <= 20; x++) tiles[x][13] = TileType::Solid;
		for (int x = 27; x <= 32; x++) tiles[x][10] = TileType::Solid;
		for (int x = 34; x <= 38; x++) tiles[x][8] = TileType::Solid; // Goal platform
	}

	// -------------------------------------------------------------
	// LEVEL 2: Wall Jumper
	// Teaches wall slide and wall jump!
	// -------------------------------------------------------------
	void Level::BuildLevel2()
	{
		levelName = "LEVEL 2: WALL JUMPER";
		levelHint = "Slide down walls to slow descent | Press SPACE against wall to Wall Jump!";
		spawnX = 64.0f;
		spawnY = static_cast<float>(TileOffsetY + 6 * TileSize - 28);
		goalX = 1184.0f;
		goalY = static_cast<float>(TileOffsetY + 4 * TileSize - 28);

		// Boundaries
		for (int x = 0; x < GridWidth; x++)
		{
			tiles[x][0] = TileType::Solid;
			tiles[x][GridHeight - 1] = TileType::Solid;
		}
		for (int y = 0; y < GridHeight; y++)
		{
			tiles[0][y] = TileType::Solid;
			tiles[GridWidth - 1][y] = TileType::Solid;
		}

		// Starting ledge up high
		for (int x = 1; x <= 6; x++) tiles[x][6] = TileType::Solid;

		// Deep shaft down
		for (int y = 7; y <= 15; y++) tiles[6][y] = TileType::Solid;
		for (int y = 10; y <= 18; y++) tiles[11][y] = TileType::Solid;

		// Spike pit at the bottom of the drop
		for (int x = 1; x <= 5; x++) tiles[x][GridHeight - 1] = TileType::SpikeUp;

		// Wall jump ascent corridor
		for (int y = 5; y <= 15; y++) tiles[16][y] = TileType::Solid;
		for (int y = 8; y <= 18; y++) tiles[22][y] = TileType::Solid;
		for (int y = 3; y <= 14; y++) tiles[28][y] = TileType::Solid;

		// Spikes under the wall jump shafts
		for (int x = 12; x <= 15; x++) tiles[x][GridHeight - 1] = TileType::SpikeUp;
		for (int x = 17; x <= 21; x++) tiles[x][GridHeight - 1] = TileType::SpikeUp;
		for (int x = 23; x <= 27; x++) tiles[x][GridHeight - 1] = TileType::SpikeUp;

		// Rest platform and final ascent
		for (int x = 29; x <= 34; x++) tiles[x][9] = TileType::Solid;
		for (int y = 2; y <= 8; y++) tiles[35][y] = TileType::Solid;
		for (int x = 36; x <= 38; x++) tiles[x][4] = TileType::Solid; // Goal platform
	}

	// -------------------------------------------------------------
	// LEVEL 3: Dash Dance
	// Introduces 8-directional dash!
	// -------------------------------------------------------------
	void Level::BuildLevel3()
	{
		levelName = "LEVEL 3: DASH DANCE";
		levelHint = "Hold Direction + J / SHIFT to DASH (8 Directions!) | Resets on ground";
		spawnX = 64.0f;
		spawnY = static_cast<float>(TileOffsetY + 16 * TileSize - 28);
		goalX = 1184.0f;
		goalY = static_cast<float>(TileOffsetY + 12 * TileSize - 28);

		// Boundaries
		for (int x = 0; x < GridWidth; x++)
		{
			tiles[x][0] = TileType::Solid;
			tiles[x][GridHeight - 1] = TileType::SpikeUp; // Bottom floor is spikes!
		}
		for (int y = 0; y < GridHeight; y++)
		{
			tiles[0][y] = TileType::Solid;
			tiles[GridWidth - 1][y] = TileType::Solid;
		}

		// Starting island
		for (int x = 1; x <= 5; x++)
		{
			tiles[x][17] = TileType::Solid;
			tiles[x][18] = TileType::Solid;
		}

		// Wide gap requiring horizontal dash
		for (int x = 11; x <= 14; x++)
		{
			tiles[x][17] = TileType::Solid;
			tiles[x][18] = TileType::Solid;
		}

		// High wall requiring Up / Up-Right dash
		for (int y = 7; y <= 18; y++)
		{
			tiles[18][y] = TileType::Solid;
		}
		// Landing over the wall
		for (int x = 19; x <= 22; x++)
		{
			tiles[x][11] = TileType::Solid;
		}

		// Long chasm with midair Dash Refill crystal!
		dashRefills.push_back({ 27.0f * TileSize, static_cast<float>(TileOffsetY + 9 * TileSize) });

		// Destination island
		for (int x = 32; x <= 34; x++)
		{
			tiles[x][10] = TileType::Solid;
		}

		// Goal ledge
		for (int x = 36; x <= 38; x++)
		{
			tiles[x][12] = TileType::Solid;
		}
	}

	// -------------------------------------------------------------
	// LEVEL 4: Crystal Chasm
	// Chains wall jumps, wall slides, and mid-air crystals
	// -------------------------------------------------------------
	void Level::BuildLevel4()
	{
		levelName = "LEVEL 4: CRYSTAL CHASM";
		levelHint = "Collect green crystals mid-air to replenish your Dash!";
		spawnX = 64.0f;
		spawnY = static_cast<float>(TileOffsetY + 16 * TileSize - 28);
		goalX = 1184.0f;
		goalY = static_cast<float>(TileOffsetY + 4 * TileSize - 28);

		// Boundaries
		for (int x = 0; x < GridWidth; x++)
		{
			tiles[x][0] = TileType::Solid;
			tiles[x][GridHeight - 1] = TileType::SpikeUp; // Spikes all across floor
		}
		for (int y = 0; y < GridHeight; y++)
		{
			tiles[0][y] = TileType::Solid;
			tiles[GridWidth - 1][y] = TileType::Solid;
		}

		// Spawn island
		for (int x = 1; x <= 4; x++)
		{
			tiles[x][17] = TileType::Solid;
			tiles[x][18] = TileType::Solid;
		}

		// Central floating pillar with spikes on sides
		for (int y = 5; y <= 15; y++)
		{
			tiles[10][y] = TileType::Solid;
		}
		// Wall slide corridor
		for (int y = 8; y <= 17; y++)
		{
			tiles[16][y] = TileType::Solid;
		}

		// Crystals in the air
		dashRefills.push_back({ 6.5f * TileSize, static_cast<float>(TileOffsetY + 11 * TileSize) });
		dashRefills.push_back({ 21.0f * TileSize, static_cast<float>(TileOffsetY + 9 * TileSize) });
		dashRefills.push_back({ 28.0f * TileSize, static_cast<float>(TileOffsetY + 5 * TileSize) });

		// Midair wall to bounce off
		for (int y = 6; y <= 13; y++)
		{
			tiles[24][y] = TileType::Solid;
		}

		// Spikes hanging from ceiling
		for (int x = 20; x <= 26; x++)
		{
			tiles[x][1] = TileType::SpikeDown;
		}

		// Final wall and goal ledge
		for (int y = 3; y <= 9; y++)
		{
			tiles[32][y] = TileType::Solid;
		}
		for (int x = 35; x <= 38; x++)
		{
			tiles[x][4] = TileType::Solid;
		}
	}

	// -------------------------------------------------------------
	// LEVEL 5: Summit Pinnacle
	// Grand finale: Springs, wall climbs, precision dash chaining!
	// -------------------------------------------------------------
	void Level::BuildLevel5()
	{
		levelName = "LEVEL 5: SUMMIT PINNACLE";
		levelHint = "Use the spring pads, wall jumps, and dash crystals to reach the peak!";
		spawnX = 64.0f;
		spawnY = static_cast<float>(TileOffsetY + 16 * TileSize - 28);
		goalX = 1184.0f;
		goalY = static_cast<float>(TileOffsetY + 3 * TileSize - 28);

		// Boundaries
		for (int x = 0; x < GridWidth; x++)
		{
			tiles[x][0] = TileType::Solid;
			tiles[x][GridHeight - 1] = TileType::SpikeUp;
		}
		for (int y = 0; y < GridHeight; y++)
		{
			tiles[0][y] = TileType::Solid;
			tiles[GridWidth - 1][y] = TileType::Solid;
		}

		// Starting platform with spring!
		for (int x = 1; x <= 5; x++)
		{
			tiles[x][17] = TileType::Solid;
			tiles[x][18] = TileType::Solid;
		}
		springs.push_back({ 4.0f * TileSize, static_cast<float>(TileOffsetY + 16 * TileSize) });

		// Tall spike wall you launch over with spring
		for (int y = 5; y <= 17; y++)
		{
			tiles[8][y] = TileType::Solid;
			tiles[7][y] = TileType::SpikeLeft;
		}

		// Air dash refill over the spike wall
		dashRefills.push_back({ 12.0f * TileSize, static_cast<float>(TileOffsetY + 4 * TileSize) });

		// Middle floating column for wall jumps
		for (int y = 4; y <= 13; y++)
		{
			tiles[17][y] = TileType::Solid;
		}
		// Right wall opposite column
		for (int y = 6; y <= 16; y++)
		{
			tiles[22][y] = TileType::Solid;
		}
		// Spikes beneath the wall jump
		for (int x = 18; x <= 21; x++)
		{
			tiles[x][GridHeight - 1] = TileType::SpikeUp;
		}

		// Another spring pad on intermediate ledge
		for (int x = 24; x <= 27; x++)
		{
			tiles[x][12] = TileType::Solid;
		}
		springs.push_back({ 26.0f * TileSize, static_cast<float>(TileOffsetY + 11 * TileSize) });

		// Ceiling spikes over the spring
		for (int x = 24; x <= 30; x++)
		{
			tiles[x][1] = TileType::SpikeDown;
		}

		// Final dash crystal before peak
		dashRefills.push_back({ 30.0f * TileSize, static_cast<float>(TileOffsetY + 6 * TileSize) });

		// High peak goal platform
		for (int x = 34; x <= 38; x++)
		{
			tiles[x][3] = TileType::Solid;
		}
		// Wall below peak
		for (int y = 4; y <= 12; y++)
		{
			tiles[33][y] = TileType::Solid;
			tiles[32][y] = TileType::SpikeLeft;
		}
	}
}
