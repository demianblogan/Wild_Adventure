#include "PatrolSystem.h"

#include "components/CollisionState.h"
#include "components/EnemyDeath.h"
#include "components/Facing.h"
#include "components/Patrol.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	PatrolSystem::PatrolSystem(Registry& registry)
		: registry(registry)
	{}

	void PatrolSystem::Update()
	{
		registry.ForEach<Patrol, Transform, Velocity>(
			[this](Entity entity, Patrol& patrol, Transform& transform, Velocity& velocity)
			{
				// EnemyDeathSystem owns the entity once it starts dying.
				if (registry.Has<EnemyDeath>(entity))
					return;

				const float position = (patrol.axis == Patrol::Axis::Horizontal) ? transform.x : transform.y;

				// Degenerate bounds (no patrolRange configured) disable the bound
				// checks: the patroller flies straight and only turns at terrain.
				if (patrol.min < patrol.max)
				{
					if (position <= patrol.min)
						patrol.direction = 1;
					else if (position >= patrol.max)
						patrol.direction = -1;
				}

				// Colliding patrollers (e.g. the blue bird) also turn around when the
				// tilemap blocks the patrol axis, so terrain shortens the configured path.
				if (registry.Has<CollisionState>(entity))
				{
					const CollisionState& cs = registry.Get<CollisionState>(entity);

					if (patrol.axis == Patrol::Axis::Horizontal)
					{
						// Only trigger on the wall the entity is actually moving toward;
						// without the direction check it re-triggers right after turning.
						if (cs.isOnWall && cs.wallDirection == patrol.direction)
							patrol.direction = -patrol.direction;
					}
					else if ((patrol.direction > 0 && cs.isOnGround)
						|| (patrol.direction < 0 && cs.isOnCeiling))
					{
						patrol.direction = -patrol.direction;
					}
				}

				const float axisVelocity = patrol.direction * patrol.speed;

				if (patrol.axis == Patrol::Axis::Horizontal)
				{
					velocity.x = axisVelocity;
					velocity.y = 0.0f;
				}
				else
				{
					velocity.x = 0.0f;
					velocity.y = axisVelocity;
				}

				if (registry.Has<Facing>(entity) && patrol.axis == Patrol::Axis::Horizontal)
					registry.Get<Facing>(entity).isLookingRight = (patrol.direction > 0);
			});
	}
}
