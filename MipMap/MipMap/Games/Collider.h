#pragma once
#include <stdint.h>
#include <vector>

namespace Games
{
	struct AABB
	{
		float x;
		float y;
		float width;
		float height;

		float Left() const { return x; }
		float Right() const { return x + width; }
		float Top() const { return y; }
		float Bottom() const { return y + height; }

		bool Overlaps(const AABB& other) const
		{
			return (Left() < other.Right() && Right() > other.Left() &&
			        Top() < other.Bottom() && Bottom() > other.Top());
		}

		bool OverlapsPoint(float px, float py) const
		{
			return (px >= Left() && px <= Right() && py >= Top() && py <= Bottom());
		}
	};

	enum class CollisionLayer : uint8_t
	{
		Player = 0,
		Ground = 1,
		Spikes = 2,
		Trigger = 3,
		Refill = 4,
		Spring = 5,
		Goal = 6
	};

	struct Collider
	{
		float xPos = 0.0f;
		float yPos = 0.0f;
		float xSize = 0.0f;
		float ySize = 0.0f;
		/// <summary>
		/// 0. Player; 1. Ground; 2. Spikes; 3. Trigger; 4. Refill; 5. Spring; 6. Goal
		/// </summary>
		uint8_t layer = 0;
		uint8_t id = 0;
		bool active = true;

		AABB GetBounds() const { return { xPos, yPos, xSize, ySize }; }
	};

	class CollisionSystem
	{
	public:
		static CollisionSystem* GetCollisionSystem();

		bool AddCollider(const Collider& collider, uint8_t& outIndex);
		bool RemoveCollider(uint8_t index);
		void UpdateCollider(uint8_t index, const Collider& collider);
		const Collider* GetCollider(uint8_t index) const;
		void Clear();

		std::vector<Collider> GetCollisions(const AABB& box, uint8_t layerMask = 0xFF) const;
		bool CheckOverlap(const AABB& box, uint8_t layerMask = 0xFF) const;

	private:
		CollisionSystem();
		~CollisionSystem() = default;

		Collider colliders[256];
		bool slotUsed[256];
		uint16_t layerMask;

		static CollisionSystem* instance;
	};
}
