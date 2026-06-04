#include "PhysicsSystem.h"

#include "components/Collider.h"
#include "components/CollisionState.h"
#include "components/Facing.h"
#include "components/Gravity.h"
#include "components/PreviousTransform.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "components/WallSlide.h"
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
				collisionState.isOnWall = false;
				collisionState.wallDirection = 0;

				transform.x += velocity.x * deltaTime;
				ResolveHorizontal(transform, collider, velocity);

				// Wall detection by touch: check for a solid tile right next to either side,
				// independent of input/velocity. Touching a wall is enough to cling.
				if (IsWallAt(transform, collider, 1))
				{
					collisionState.isOnWall = true;
					collisionState.wallDirection = 1;
				}
				else if (IsWallAt(transform, collider, -1))
				{
					collisionState.isOnWall = true;
					collisionState.wallDirection = -1;
				}

				// Clinging to a wall caps the fall speed (slow slide).
				if (registry.Has<WallSlide>(entity) && collisionState.isOnWall && velocity.y > 0.0f)
				{
					const WallSlide& wallSlide = registry.Get<WallSlide>(entity);
					if (velocity.y > wallSlide.slideSpeed)
						velocity.y = wallSlide.slideSpeed;
				}

				transform.y += velocity.y * deltaTime;
				ResolveVertical(transform, collider, velocity, collisionState);

				if (registry.Has<Facing>(entity))
				{
					Facing& facing = registry.Get<Facing>(entity);

					// While clinging to a wall, face the wall so the slide animation
					// is oriented correctly no matter how we approached it.
					if (collisionState.isOnWall && !collisionState.isOnGround)
						facing.isLookingRight = (collisionState.wallDirection > 0);
					else if (velocity.x != 0.0f)
						facing.isLookingRight = velocity.x > 0.0f;
				}
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

		if (velocity.y > 0.0f)
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
		else
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

	bool PhysicsSystem::IsWallAt(const Transform& transform, const Collider& collider, int direction) const
	{
		const float tileSize = static_cast<float>(tilemap.tileSize);
		const float halfWidth = collider.width / 2.0f;

		const float top = transform.y - collider.height;
		const float bottom = transform.y;

		const int firstRow = static_cast<int>(std::floor(top / tileSize));
		const int lastRow = static_cast<int>(std::floor((bottom - 0.001f) / tileSize));

		const float edge = (direction > 0) ? (transform.x + halfWidth) : (transform.x - halfWidth);
		const int column = static_cast<int>(std::floor((edge + direction * 1.0f) / tileSize));

		for (int row = firstRow; row <= lastRow; row++)
		{
			if (tilemap.IsSolid(column, row))
				return true;
		}

		return false;
	}
}