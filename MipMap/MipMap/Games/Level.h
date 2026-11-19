#pragma once
#include "Engine/Renderer/DirectX/DX12Renderer.h"
#include "Collider.h"
#include "Particles.h"
#include <vector>
#include <string>

namespace Games
{
	enum class TileType : uint8_t
	{
		Empty = 0,
		Solid = 1,
		Platform = 2,
		SpikeUp = 3,
		SpikeDown = 4,
		SpikeLeft = 5,
		SpikeRight = 6,
		DashRefill = 7,
		Spring = 8,
		Goal = 9
	};

	struct DashRefillEntity
	{
		float x;
		float y;
		bool active = true;
		float respawnTimer = 0.0f;
		float animTimer = 0.0f;
	};

	struct SpringEntity
	{
		float x;
		float y;
		float bounceTimer = 0.0f;
	};

	class Level
	{
	public:
		static const int GridWidth = 40;
		static const int GridHeight = 20;
		static const int TileSize = 32;
		static const int TileOffsetY = 40;

		Level();

		void LoadLevel(int levelIndex);
		void Reset();
		void Update(float dt, ParticleSystem& particles);
		void Render(Renderer::DX12Renderer* renderer, ParticleSystem& particles);

		bool IsSolidAt(int gridX, int gridY) const;
		bool CheckSolidOverlap(const AABB& box) const;
		bool CheckPlatformOverlap(const AABB& box, float prevBottom) const;
		bool CheckHazardOverlap(const AABB& box) const;

		// Interactive entity checks
		bool CheckDashRefillOverlap(const AABB& box, ParticleSystem& particles);
		bool CheckSpringOverlap(const AABB& box, float& outBounceVy);
		bool CheckGoalOverlap(const AABB& box) const;

		float GetSpawnX() const { return spawnX; }
		float GetSpawnY() const { return spawnY; }
		const std::string& GetName() const { return levelName; }
		const std::string& GetHint() const { return levelHint; }
		int GetCurrentIndex() const { return currentLevelIndex; }
		int GetTotalLevels() const { return totalLevels; }

	private:
		int currentLevelIndex = 1;
		int totalLevels = 5;
		std::string levelName;
		std::string levelHint;
		float spawnX = 64.0f;
		float spawnY = 500.0f;
		float goalX = 1180.0f;
		float goalY = 500.0f;
		float goalAnim = 0.0f;

		TileType tiles[GridWidth][GridHeight];
		std::vector<DashRefillEntity> dashRefills;
		std::vector<SpringEntity> springs;

		void ClearGrid();
		void BuildLevel1();
		void BuildLevel2();
		void BuildLevel3();
		void BuildLevel4();
		void BuildLevel5();
	};
}
