#include "GroundPatrolSystem.h"

#include "components/AnimationState.h"
#include "components/Collider.h"
#include "components/CollisionState.h"
#include "components/EnemyDeath.h"
#include "components/Facing.h"
#include "components/GroundPatrol.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <cmath>

namespace ECS
{
	GroundPatrolSystem::GroundPatrolSystem(Registry& registry, const Tilemap& tilemap)
		: registry(registry)
		, tilemap(tilemap)
	{}

	void GroundPatrolSystem::Update(float deltaTime)
	{
		registry.ForEach<GroundPatrol, Transform, Collider, Velocity, AnimationState>(
			[&](Entity entity, GroundPatrol& patrol, Transform& transform,
				Collider& collider, Velocity& velocity, AnimationState& animState)
			{
				// EnemyDeathSystem owns the entity once it starts dying.
				if (registry.Has<EnemyDeath>(entity))
					return;

				switch (patrol.state)
				{
				case GroundPatrol::State::Patrolling:
				{
					velocity.x = patrol.speed * static_cast<float>(patrol.direction);

					bool shouldTurn = false;

					if (registry.Has<CollisionState>(entity))
					{
						const CollisionState& cs = registry.Get<CollisionState>(entity);
						// Only trigger on the wall the entity is actually walking toward;
						// without the direction check it re-triggers immediately after turning.
						if (cs.isOnWall && cs.wallDirection == patrol.direction)
							shouldTurn = true;
					}

					if (!shouldTurn && !HasGroundAhead(transform, collider, patrol.direction))
						shouldTurn = true;

					if (shouldTurn)
					{
						patrol.state      = GroundPatrol::State::TurningIdle;
						patrol.stateTimer = GroundPatrol::TURN_IDLE_DURATION;
						velocity.x        = 0.0f;
						animState.current = "Idle";
					}
					break;
				}

				case GroundPatrol::State::TurningIdle:
					velocity.x = 0.0f;
					patrol.stateTimer -= deltaTime;
					if (patrol.stateTimer <= 0.0f)
					{
						patrol.direction  = -patrol.direction;
						patrol.state      = GroundPatrol::State::Patrolling;
						animState.current = "Run";
					}
					break;
				}

				// PhysicsSystem zeros velocity.x for non-player entities before its own Facing
				// update, so update Facing explicitly here based on the patrol direction.
				if (registry.Has<Facing>(entity))
					registry.Get<Facing>(entity).isLookingRight = (patrol.direction > 0);
			});
	}

	bool GroundPatrolSystem::HasGroundAhead(
		const Transform& transform, const Collider& collider, int direction) const
	{
		const float tileSize = static_cast<float>(tilemap.tileSize);
		const float probeX   = transform.x + static_cast<float>(direction) * (collider.width / 2.0f + 2.0f);
		const float probeY   = transform.y + tileSize * 0.5f;
		const int   col      = static_cast<int>(std::floor(probeX / tileSize));
		const int   row      = static_cast<int>(std::floor(probeY / tileSize));
		return tilemap.IsSolid(col, row);
	}
}
