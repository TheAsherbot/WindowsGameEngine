#pragma once
#include "Engine/Renderer/DirectX/DX12Renderer.h"
#include <vector>

namespace Games
{
	struct Particle
	{
		float x = 0.0f;
		float y = 0.0f;
		float vx = 0.0f;
		float vy = 0.0f;
		float size = 2.0f;
		float life = 1.0f;
		float maxLife = 1.0f;
		Renderer::Color color;
		bool shrink = true;
	};

	struct DashGhost
	{
		float x = 0.0f;
		float y = 0.0f;
		float width = 28.0f;
		float height = 28.0f;
		float life = 0.25f;
		float maxLife = 0.25f;
		Renderer::Color color;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem() = default;

		void Update(float dt);
		void Render(Renderer::DX12Renderer* renderer);
		void Clear();

		void SpawnDust(float x, float y, float dirX, float dirY, int count = 6, Renderer::Color color = Renderer::Color(0.85f, 0.85f, 0.9f, 0.8f));
		void SpawnJumpDust(float x, float y, float width);
		void SpawnWallSlideDust(float x, float y, bool onRightWall);
		void SpawnDeathBurst(float x, float y);
		void SpawnCrystalCollect(float x, float y);
		void SpawnPortalSparkles(float x, float y, float radius);
		void AddGhost(float x, float y, float w, float h, Renderer::Color color);

	private:
		std::vector<Particle> particles;
		std::vector<DashGhost> ghosts;
	};
}
