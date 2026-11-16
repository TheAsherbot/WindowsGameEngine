#include "Collider.h"

namespace Games
{
	static CollisionSystem* CollisionSystem::GetCollisionSystem()
	{
		if (instance == nullptr)
		{
			instance = new CollisionSystem();
		}

		return instance;
	}

	const bool CollisionSystem::AddCollider(Collider& collider)
	{
		if (collider.id == 0)
		{
			for (uint8_t i = 1; i < 256; i++)
			{
				if (colliders[i].id == 0)
				{
					colliders[i] = collider;
					colliders[i].id = i;
					return true;
				}
			}
		}
		else
		{
			if (colliders[collider.id].id = 0)
			{
				colliders[i] = collider;
				return true;
			}
		}
		
		return false;
	}
	const bool CollisionSystem::RemoveCollider(Collider collider)
	{
		RemoveCollider(collider.id)
	}
	const bool CollisionSystem::RemoveCollider(uint8_t& index)
	{
		if (index == 0)
		{
			return false;
		}

		colliders[index] = colliders[0];
	}



	void CollisionSystem::UpdateCollider(Collider& collider, uint8_t& index)
	{
		if (index == 0)
		{
			return;
		}

		colliders[index] = collider;
		colliders[index].id = index;
	}
	void CollisionSystem::UpdateCollider(Collider& collider)
	{
		UpdateCollider(collider, collider.id);
	}
	Collider* CollisionSystem::GetCollider(uint8_t& index) const
	{
		return colliders[index];
	}


	const Collider* CollisionSystem::GetCollisions(Collider& collider, uint8_t& collisionCount) const
	{
		Collider collidersHit[collisionCount];
		uint8_t hitIndex = 0;
		
		float maxX = collider.xPos + collider.xSize / 2;
		float maxY = collider.yPos + collider.ySize / 2;
		float minX = collider.xPos - collider.xSize / 2;
		float minY = collider.yPos - collider.ySize / 2;

		for (uint8_t i = 1; i < 256; i++)
		{
			if (colliders[i].id == collider.id)
			{
				continue;
			}
			if ((1 << (collider.layer * 4 + colliders[i].id)) & layerMask)
			{
				continue;
			}

			float colliderMaxX = collider.xPos + collider.xSize / 2;
			float colliderMaxY = collider.yPos + collider.ySize / 2;
			float colliderMinX = collider.xPos - collider.xSize / 2;
			float colliderMinY = collider.yPos - collider.ySize / 2;


			if (((colliderMinY < maxY && colliderMinY > maxY) && ((colliderMinX < maxX && colliderMinX > maxX) || (colliderMaxX < maxX && colliderMaxX > maxX))) ||
				((colliderMaxY < maxY && colliderMaxY > maxY) && ((colliderMinX < maxX && colliderMinX > maxX) || (colliderMaxX < maxX && colliderMaxX > maxX))))
			{
				collidersHit[hitIndex] = colliders[i];
				hitIndex++;
				if (collisionCount <= hitIndex)
				{
					break;
				}
			}
			else
			{
				//no hit.
				continue;
			}
		}

		collisionCount = hitIndex;
		return collidersHit;
	}

	void CollisionSystem::RemoveLayerContact(uint8_t layer1, uint_t layer2)
	{
		layerMask = layermask | 1 << (layer1 * 4 + layer2);
	}
	void CollisionSystem::AddLayerContact(uint8_t layer1, uint_t layer2)
	{
		layerMask = layermask & ~(1 << (layer1 * 4 + layer2));
	}


	CollisionSystem::CollisionSystem()
	{
		Collider collider();
		collider.xPos = 0;
		collider.yPos = 0;
		collider.xSize = 0;
		collider.ySize = 0;
		collider.id = 0;
		collider.layer = 0;

		instance = this;
	}
}