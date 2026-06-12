#include "PhysicsSystem.h"

#include "components/Box.h"
#include "components/BoxHit.h"
#include "components/Collider.h"
#include "components/CollisionState.h"
#include "components/Facing.h"
#include "components/Gravity.h"
#include "components/Jump.h"
#include "components/PreviousTransform.h"
#include "components/Solid.h"
#include "components/Trampoline.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "components/WallSlide.h"
#include "components/Health.h"
#include "components/Player.h"
#include "components/Frozen.h"
#include "core/ecs/Registry.h"
#include "tilemap/Tilemap.h"

#include <cmath>
#include <vector>

namespace ECS
{
	PhysicsSystem::PhysicsSystem(Registry& registry, const Tilemap& tilemap)
		: registry(registry)
		, tilemap(tilemap)
	{}

	void PhysicsSystem::Update(float deltaTime)
	{
		std::vector<SolidBox> boxes;
		registry.ForEach<Solid, Transform>(
			[&boxes](Entity entity, Solid& solid, Transform& transform)
			{
				const float halfWidth = solid.width / 2.0f;
				const float centerX = transform.x + solid.offsetX;
				const float bottom = transform.y + solid.offsetY;
				boxes.push_back({ entity, centerX - halfWidth, centerX + halfWidth,
					bottom - solid.height, bottom, solid.bounceSpeed });
			});

		registry.ForEach<Transform, Velocity, Collider, Gravity, CollisionState>(
			[this, deltaTime, &boxes](Entity entity, Transform& transform, Velocity& velocity,
				Collider& collider, Gravity& gravity, CollisionState& collisionState)
			{
				if (registry.Has<Frozen>(entity))
					return;

				if (registry.Has<PreviousTransform>(entity))
				{
					PreviousTransform& previous = registry.Get<PreviousTransform>(entity);
					previous.x = transform.x;
					previous.y = transform.y;
				}

				const bool dead = registry.Has<Health>(entity) && registry.Get<Health>(entity).current <= 0;
				const bool collidesWithBoxes = registry.Has<Player>(entity);

				velocity.y += gravity.acceleration * deltaTime;
				if (velocity.y > gravity.maxFallSpeed)
					velocity.y = gravity.maxFallSpeed;

				collisionState.isOnGround = false;
				collisionState.isOnCeiling = false;
				collisionState.isOnWall = false;
				collisionState.wallDirection = 0;

				transform.x += velocity.x * deltaTime;
				if (!dead)
				{
					ResolveHorizontal(transform, collider, velocity);
					if (collidesWithBoxes)
						ResolveHorizontalBoxes(transform, collider, velocity, boxes);
				}

				if (!dead)
				{
					const bool wallRight = IsWallAt(transform, collider, 1)
						|| (collidesWithBoxes && IsBoxWallAt(transform, collider, 1, boxes));
					const bool wallLeft = IsWallAt(transform, collider, -1)
						|| (collidesWithBoxes && IsBoxWallAt(transform, collider, -1, boxes));

					if (wallRight)
					{
						collisionState.isOnWall = true;
						collisionState.wallDirection = 1;
					}
					else if (wallLeft)
					{
						collisionState.isOnWall = true;
						collisionState.wallDirection = -1;
					}

					if (registry.Has<WallSlide>(entity) && collisionState.isOnWall && velocity.y > 0.0f)
					{
						const WallSlide& wallSlide = registry.Get<WallSlide>(entity);
						if (velocity.y > wallSlide.slideSpeed)
							velocity.y = wallSlide.slideSpeed;
					}
				}

				transform.y += velocity.y * deltaTime;
				if (!dead)
				{
					ResolveVertical(transform, collider, velocity, collisionState);
					if (collidesWithBoxes)
						ResolveVerticalBoxes(entity, transform, collider, velocity, collisionState, boxes);
				}

				// Dropped items (no player control) stop sliding once they land.
				if (!collidesWithBoxes && collisionState.isOnGround)
					velocity.x = 0.0f;

				if (registry.Has<Facing>(entity))
				{
					Facing& facing = registry.Get<Facing>(entity);

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
					collisionState.isOnCeiling = true;
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

	void PhysicsSystem::ResolveHorizontalBoxes(Transform& transform, const Collider& collider, Velocity& velocity, const std::vector<SolidBox>& boxes) const
	{
		if (velocity.x == 0.0f)
			return;

		const float halfWidth = collider.width / 2.0f;
		const float top = transform.y - collider.height;
		const float bottom = transform.y;

		for (const SolidBox& box : boxes)
		{
			const float left = transform.x - halfWidth;
			const float right = transform.x + halfWidth;

			const bool overlap = left < box.right && right > box.left && top < box.bottom && bottom > box.top;
			if (!overlap)
				continue;

			if (velocity.x > 0.0f)
				transform.x = box.left - halfWidth;
			else
				transform.x = box.right + halfWidth;

			velocity.x = 0.0f;
		}
	}

	void PhysicsSystem::ResolveVerticalBoxes(Entity entity, Transform& transform, const Collider& collider, Velocity& velocity, CollisionState& collisionState, const std::vector<SolidBox>& boxes)
	{
		if (velocity.y == 0.0f)
			return;

		const float halfWidth = collider.width / 2.0f;

		for (const SolidBox& box : boxes)
		{
			const float left = transform.x - halfWidth;
			const float right = transform.x + halfWidth;
			const float top = transform.y - collider.height;
			const float bottom = transform.y;

			const bool overlap = left < box.right && right > box.left && top < box.bottom && bottom > box.top;
			if (!overlap)
				continue;

			if (velocity.y > 0.0f) // landed on the box top
			{
				transform.y = box.top;

				if (box.bounceSpeed > 0.0f)
				{
					velocity.y = -box.bounceSpeed; // trampoline

					// The bounce acts as a first jump: guarantee one air jump after it,
					// which then plays the double-jump sound and animation.
					if (registry.Has<Jump>(entity) && registry.Get<Jump>(entity).jumpsRemaining < 1)
						registry.Get<Jump>(entity).jumpsRemaining = 1;

					if (registry.Has<Trampoline>(box.entity))
						registry.Get<Trampoline>(box.entity).wasBounced = true;
				}
				else
				{
					velocity.y = 0.0f;
					collisionState.isOnGround = true;
				}
			}
			else // hit the box from below
			{
				transform.y = box.bottom + collider.height;
				velocity.y = 0.0f;
			}

			if (!registry.Has<BoxHit>(box.entity) && registry.Has<Box>(box.entity))
				registry.Add<BoxHit>(box.entity, {});
		}
	}

	bool PhysicsSystem::IsBoxWallAt(const Transform& transform, const Collider& collider, int direction, const std::vector<SolidBox>& boxes) const
	{
		const float halfWidth = collider.width / 2.0f;
		const float top = transform.y - collider.height;
		const float bottom = transform.y;

		const float edge = (direction > 0) ? (transform.x + halfWidth) : (transform.x - halfWidth);
		const float probe = edge + direction * 1.0f;

		for (const SolidBox& box : boxes)
		{
			const bool verticalOverlap = top < box.bottom && (bottom - 0.001f) > box.top;
			if (verticalOverlap && probe >= box.left && probe <= box.right)
				return true;
		}

		return false;
	}
}