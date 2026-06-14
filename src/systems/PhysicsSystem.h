#pragma once

#include "core/ecs/Entity.h"

#include <vector>

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

		// Multiplies gravity for the whole level (e.g. a water level is floatier).
		void SetGravityScale(float scale) { gravityScale = scale; }

	private:
		struct SolidBox
		{
			Entity entity;
			float left;
			float right;
			float top;
			float bottom;
			float bounceSpeed;
		};

		void ResolveHorizontal(Transform& transform, const Collider& collider, Velocity& velocity) const;
		void ResolveVertical(Transform& transform, const Collider& collider, Velocity& velocity, CollisionState& collisionState) const;
		bool IsWallAt(const Transform& transform, const Collider& collider, int direction) const;

		void ResolveHorizontalBoxes(Transform& transform, const Collider& collider, Velocity& velocity, const std::vector<SolidBox>& boxes) const;
		void ResolveVerticalBoxes(Entity entity, Transform& transform, const Collider& collider, Velocity& velocity, CollisionState& collisionState, const std::vector<SolidBox>& boxes);
		bool IsBoxWallAt(const Transform& transform, const Collider& collider, int direction, const std::vector<SolidBox>& boxes) const;

		Registry& registry;
		const Tilemap& tilemap;
		float gravityScale = 1.0f;
	};
}