#pragma once

struct Tilemap;

namespace ECS
{
	class Registry;
	struct Transform;
	struct Velocity;
	struct Collider;
	struct CollisionState;

	class PhysicsSystem
	{
	public:
		PhysicsSystem(Registry& registry, const Tilemap& tilemap);

		void Update(float deltaTime);

	private:
		void ResolveHorizontal(Transform& transform, const Collider& collider, Velocity& velocity) const;
		void ResolveVertical(Transform& transform, const Collider& collider, Velocity& velocity, CollisionState& collisionState) const;

		Registry& registry;
		const Tilemap& tilemap;
	};
}