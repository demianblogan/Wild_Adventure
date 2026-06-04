#include "PhysicsSystem.h"

#include "components/Collider.h"
#include "components/CollisionState.h"
#include "components/Facing.h"
#include "components/Gravity.h"
#include "components/PreviousTransform.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"
#include "tilemap/Tilemap.h"

#include <cmath>

namespace ECS
{
	PhysicsSystem::PhysicsSystem(Registry& registry, const Tilemap& tilemap)
		: registry(registry)
		, tilemap(tilemap)
	{}

	void PhysicsSystem::Update(float deltaTime)
	{
		registry.ForEach<Transform, Velocity, Collider, Gravity, CollisionState>(
			[this, deltaTime](Entity entity, Transform& transform, Velocity& velocity,
				Collider& collider, Gravity& gravity, CollisionState& collisionState)
			{
				if (registry.Has<PreviousTransform>(entity))
				{
					PreviousTransform& previous = registry.Get<PreviousTransform>(entity);
					previous.x = transform.x;
					previous.y = transform.y;
				}

				velocity.y += gravity.acceleration * deltaTime;
				if (velocity.y > gravity.maxFallSpeed)
					velocity.y = gravity.maxFallSpeed;

				collisionState.isOnGround = false;

				// Move and resolve one axis at a time: this keeps corners and walls clean.
				transform.x += velocity.x * deltaTime;
				ResolveHorizontal(transform, collider, velocity);

				transform.y += velocity.y * deltaTime;
				ResolveVertical(transform, collider, velocity, collisionState);

				if (registry.Has<Facing>(entity) && velocity.x != 0.0f)
					registry.Get<Facing>(entity).isLookingRight = velocity.x > 0.0f;
			});
	}

	void PhysicsSystem::ResolveHorizontal(Transform& transform, const Collider& collider, Velocity& velocity) const
	{
		if (velocity.x == 0.0f)
			return;

		const float tileSize = static_cast<float>(tilemap.tileSize);
		const float halfWidth = collider.width / 2.0f;

		const float top = transform.y - collider.height;
		const float bottom = transform.y;

		// Rows the body spans. The tiny offset on the bottom keeps the floor tile
		// we stand on from counting as a side wall.
		const int firstRow = static_cast<int>(std::floor(top / tileSize));
		const int lastRow = static_cast<int>(std::floor((bottom - 0.001f) / tileSize));

		if (velocity.x > 0.0f)
		{
			const float right = transform.x + halfWidth;
			const int column = static_cast<int>(std::floor(right / tileSize));

			for (int row = firstRow; row <= lastRow; row++)
			{
				if (tilemap.IsSolid(column, row))
				{
					transform.x = column * tileSize - halfWidth;
					velocity.x = 0.0f;
					break;
				}
			}
		}
		else
		{
			const float left = transform.x - halfWidth;
			const int column = static_cast<int>(std::floor(left / tileSize));

			for (int row = firstRow; row <= lastRow; row++)
			{
				if (tilemap.IsSolid(column, row))
				{
					transform.x = (column + 1) * tileSize + halfWidth;
					velocity.x = 0.0f;
					break;
				}
			}
		}
	}

	void PhysicsSystem::ResolveVertical(Transform& transform, const Collider& collider, Velocity& velocity, CollisionState& collisionState) const
	{
		if (velocity.y == 0.0f)
			return;

		const float tileSize = static_cast<float>(tilemap.tileSize);
		const float halfWidth = collider.width / 2.0f;

		const float left = transform.x - halfWidth;
		const float right = transform.x + halfWidth;

		const int firstColumn = static_cast<int>(std::floor(left / tileSize));
		const int lastColumn = static_cast<int>(std::floor((right - 0.001f) / tileSize));

		if (velocity.y > 0.0f) // moving down
		{
			const float bottom = transform.y;
			const int row = static_cast<int>(std::floor(bottom / tileSize));

			for (int column = firstColumn; column <= lastColumn; column++)
			{
				if (tilemap.IsSolid(column, row))
				{
					transform.y = row * tileSize;
					velocity.y = 0.0f;
					collisionState.isOnGround = true;
					break;
				}
			}
		}
		else // moving up
		{
			const float top = transform.y - collider.height;
			const int row = static_cast<int>(std::floor(top / tileSize));

			for (int column = firstColumn; column <= lastColumn; column++)
			{
				if (tilemap.IsSolid(column, row))
				{
					transform.y = (row + 1) * tileSize + collider.height;
					velocity.y = 0.0f;
					break;
				}
			}
		}
	}
}