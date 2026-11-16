#include <stdint.h>

namespace Games
{
	struct Collider
	{
		float xPos;
		float yPos;
		float xSize;
		float ySize;
		/// <summary>
		/// 0. Player; 2. Static Ground; 3. Spikes; 4. Trigger
		/// </summary>
		uint8_t layer;
		uint8_t id;
	};

	class CollisionSystem
	{
	public:
		static CollisionSystem* GetCollisionSystem();

		const bool AddCollider(Collider& collider, uint8_t& index);
		const bool RemoveCollider(Collider collider);
		const bool RemoveCollider(uint8_t& index);

		void UpdateCollider(Collider& collider, uint8_t& index);
		void UpdateCollider(Collider& collider);
		const Collider* GetCollider(uint8_t& index) const;

		const Collider* GetCollisions(Collider& collider, uint8_t& collisionCount) const;

		void RemoveLayerContact(uint8_t layer1, uint_t layer2);
		void AddLayerContact(uint8_t layer1, uint_t layer2);

	private:
		CollisionSystem();
		~CollisionSystem();

		Collider colliders[256];
		/// <summary>
		///    0123
		///  _________
		/// 0| 1000
		/// 1| 0111
		/// 2| 0111
		/// 3| 0111
		/// 
		/// matrix like that for the four layers 0 means collide, 1 means ignore it. If 
		/// </summary>
		ushort layerMast;

		static CollisionSystem* instance = nullptr;
	};
}