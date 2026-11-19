#include "Collider.h"

namespace Games
{
	CollisionSystem* CollisionSystem::instance = nullptr;

	CollisionSystem* CollisionSystem::GetCollisionSystem()
	{
		if (instance == nullptr)
		{
			instance = new CollisionSystem();
		}
		return instance;
	}

	CollisionSystem::CollisionSystem()
		: layerMask(0xFFFF)
	{
		for (int i = 0; i < 256; i++)
		{
			slotUsed[i] = false;
			colliders[i].id = static_cast<uint8_t>(i);
			colliders[i].active = false;
		}
	}

	bool CollisionSystem::AddCollider(const Collider& collider, uint8_t& outIndex)
	{
		for (int i = 1; i < 256; i++)
		{
			if (!slotUsed[i])
			{
				slotUsed[i] = true;
				colliders[i] = collider;
				colliders[i].id = static_cast<uint8_t>(i);
				colliders[i].active = true;
				outIndex = static_cast<uint8_t>(i);
				return true;
			}
		}
		return false;
	}

	bool CollisionSystem::RemoveCollider(uint8_t index)
	{
		if (index == 0 || !slotUsed[index])
		{
			return false;
		}
		slotUsed[index] = false;
		colliders[index].active = false;
		return true;
	}

	void CollisionSystem::UpdateCollider(uint8_t index, const Collider& collider)
	{
		if (index > 0 && slotUsed[index])
		{
			colliders[index] = collider;
			colliders[index].id = index;
		}
	}

	const Collider* CollisionSystem::GetCollider(uint8_t index) const
	{
		if (index > 0 && slotUsed[index])
		{
			return &colliders[index];
		}
		return nullptr;
	}

	void CollisionSystem::Clear()
	{
		for (int i = 1; i < 256; i++)
		{
			slotUsed[i] = false;
			colliders[i].active = false;
		}
	}

	std::vector<Collider> CollisionSystem::GetCollisions(const AABB& box, uint8_t mask) const
	{
		std::vector<Collider> hits;
		hits.reserve(16);

		for (int i = 1; i < 256; i++)
		{
			if (!slotUsed[i] || !colliders[i].active)
			{
				continue;
			}

			if (!(mask & (1 << colliders[i].layer)))
			{
				continue;
			}

			if (box.Overlaps(colliders[i].GetBounds()))
			{
				hits.push_back(colliders[i]);
			}
		}

		return hits;
	}

	bool CollisionSystem::CheckOverlap(const AABB& box, uint8_t mask) const
	{
		for (int i = 1; i < 256; i++)
		{
			if (!slotUsed[i] || !colliders[i].active)
			{
				continue;
			}

			if (!(mask & (1 << colliders[i].layer)))
			{
				continue;
			}

			if (box.Overlaps(colliders[i].GetBounds()))
			{
				return true;
			}
		}
		return false;
	}
}
