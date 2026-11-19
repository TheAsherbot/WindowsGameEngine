#include "Particles.h"
#include <cstdlib>
#include <cmath>

namespace Games
{
	static float RandomFloat(float min, float max)
	{
		float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		return min + r * (max - min);
	}

	void ParticleSystem::Update(float dt)
	{
		// Update particles
		for (size_t i = 0; i < particles.size();)
		{
			particles[i].life -= dt;
			if (particles[i].life <= 0.0f)
			{
				particles[i] = particles.back();
				particles.pop_back();
			}
			else
			{
				particles[i].x += particles[i].vx * dt;
				particles[i].y += particles[i].vy * dt;
				// Slight gravity & drag
				particles[i].vy += 120.0f * dt;
				particles[i].vx *= (1.0f - 2.0f * dt);
				i++;
			}
		}

		// Update dash ghosts
		for (size_t i = 0; i < ghosts.size();)
		{
			ghosts[i].life -= dt;
			if (ghosts[i].life <= 0.0f)
			{
				ghosts[i] = ghosts.back();
				ghosts.pop_back();
			}
			else
			{
				i++;
			}
		}
	}

	void ParticleSystem::Render(Renderer::DX12Renderer* renderer)
	{
		// Render dash ghost afterimages
		for (const auto& ghost : ghosts)
		{
			float alpha = ghost.life / ghost.maxLife;
			Renderer::Color c = ghost.color;
			c.a *= alpha * 0.45f;
			int x1 = static_cast<int>(ghost.x);
			int y1 = static_cast<int>(ghost.y);
			int x2 = static_cast<int>(ghost.x + ghost.width);
			int y2 = static_cast<int>(ghost.y + ghost.height);
			renderer->DrawFilledRectangle(x1, y1, x2, y2, c);
			renderer->DrawRectangle(x1, y1, x2, y2, Renderer::Color(c.r, c.g, c.b, c.a * 1.2f));
		}

		// Render particles
		for (const auto& p : particles)
		{
			float ratio = p.life / p.maxLife;
			float s = p.shrink ? (p.size * ratio) : p.size;
			if (s < 1.0f) s = 1.0f;

			Renderer::Color c = p.color;
			c.a *= ratio;

			int x1 = static_cast<int>(p.x - s);
			int y1 = static_cast<int>(p.y - s);
			int x2 = static_cast<int>(p.x + s);
			int y2 = static_cast<int>(p.y + s);
			renderer->DrawFilledRectangle(x1, y1, x2, y2, c);
		}
	}

	void ParticleSystem::Clear()
	{
		particles.clear();
		ghosts.clear();
	}

	void ParticleSystem::AddGhost(float x, float y, float w, float h, Renderer::Color color)
	{
		DashGhost g;
		g.x = x;
		g.y = y;
		g.width = w;
		g.height = h;
		g.life = 0.22f;
		g.maxLife = 0.22f;
		g.color = color;
		ghosts.push_back(g);
	}

	void ParticleSystem::SpawnDust(float x, float y, float dirX, float dirY, int count, Renderer::Color color)
	{
		for (int i = 0; i < count; i++)
		{
			Particle p;
			p.x = x + RandomFloat(-3.0f, 3.0f);
			p.y = y + RandomFloat(-3.0f, 3.0f);
			p.vx = dirX * RandomFloat(20.0f, 90.0f) + RandomFloat(-30.0f, 30.0f);
			p.vy = dirY * RandomFloat(20.0f, 90.0f) + RandomFloat(-40.0f, 10.0f);
			p.size = RandomFloat(2.0f, 4.0f);
			p.life = RandomFloat(0.2f, 0.45f);
			p.maxLife = p.life;
			p.color = color;
			particles.push_back(p);
		}
	}

	void ParticleSystem::SpawnJumpDust(float x, float y, float width)
	{
		SpawnDust(x + 4.0f, y, -1.0f, -0.2f, 5);
		SpawnDust(x + width - 4.0f, y, 1.0f, -0.2f, 5);
	}

	void ParticleSystem::SpawnWallSlideDust(float x, float y, bool onRightWall)
	{
		float dirX = onRightWall ? -1.0f : 1.0f;
		Particle p;
		p.x = x;
		p.y = y + RandomFloat(-4.0f, 4.0f);
		p.vx = dirX * RandomFloat(15.0f, 45.0f);
		p.vy = RandomFloat(-25.0f, -5.0f);
		p.size = RandomFloat(1.5f, 3.0f);
		p.life = RandomFloat(0.15f, 0.35f);
		p.maxLife = p.life;
		p.color = Renderer::Color(0.85f, 0.85f, 0.95f, 0.7f);
		particles.push_back(p);
	}

	void ParticleSystem::SpawnDeathBurst(float x, float y)
	{
		// Dramatic death explosion of 30 colored pixel cubes
		Renderer::Color colors[] = {
			Renderer::Color(1.0f, 0.2f, 0.3f, 1.0f),
			Renderer::Color(1.0f, 0.6f, 0.1f, 1.0f),
			Renderer::Color(0.2f, 0.8f, 1.0f, 1.0f),
			Renderer::Color(1.0f, 1.0f, 1.0f, 1.0f),
			Renderer::Color(0.9f, 0.1f, 0.5f, 1.0f)
		};

		for (int i = 0; i < 36; i++)
		{
			float angle = RandomFloat(0.0f, 6.283185f);
			float speed = RandomFloat(80.0f, 420.0f);
			Particle p;
			p.x = x;
			p.y = y;
			p.vx = std::cos(angle) * speed;
			p.vy = std::sin(angle) * speed;
			p.size = RandomFloat(3.0f, 6.0f);
			p.life = RandomFloat(0.35f, 0.75f);
			p.maxLife = p.life;
			p.color = colors[rand() % 5];
			particles.push_back(p);
		}
	}

	void ParticleSystem::SpawnCrystalCollect(float x, float y)
	{
		for (int i = 0; i < 20; i++)
		{
			float angle = RandomFloat(0.0f, 6.283185f);
			float speed = RandomFloat(50.0f, 250.0f);
			Particle p;
			p.x = x;
			p.y = y;
			p.vx = std::cos(angle) * speed;
			p.vy = std::sin(angle) * speed;
			p.size = RandomFloat(2.5f, 5.0f);
			p.life = RandomFloat(0.3f, 0.6f);
			p.maxLife = p.life;
			p.color = Renderer::Color(0.2f, 1.0f, 0.6f, 0.9f);
			particles.push_back(p);
		}
	}

	void ParticleSystem::SpawnPortalSparkles(float x, float y, float radius)
	{
		if (rand() % 3 != 0) return;
		float angle = RandomFloat(0.0f, 6.283185f);
		float dist = RandomFloat(0.0f, radius);
		Particle p;
		p.x = x + std::cos(angle) * dist;
		p.y = y + std::sin(angle) * dist;
		p.vx = -std::cos(angle) * RandomFloat(10.0f, 30.0f);
		p.vy = -std::sin(angle) * RandomFloat(10.0f, 30.0f);
		p.size = RandomFloat(1.5f, 3.5f);
		p.life = RandomFloat(0.4f, 0.8f);
		p.maxLife = p.life;
		p.color = Renderer::Color(0.5f, 0.8f, 1.0f, 0.8f);
		particles.push_back(p);
	}
}
