#include "EnemyDeathSystem.h"

#include "components/combat/EnemyDeath.h"
#include "components/physics/Gravity.h"
#include "components/combat/Health.h"
#include "components/physics/Rotation.h"
#include "components/physics/Transform.h"
#include "components/physics/Velocity.h"
#include "core/Random.h"
#include "core/ecs/Registry.h"

#include <vector>

namespace ECS
{
	EnemyDeathSystem::EnemyDeathSystem(Registry& registry)
		: registry(registry)
	{}

	void EnemyDeathSystem::Update(float deltaTime, float fallLimit)
	{
		std::vector<Entity> toDestroy;

		registry.ForEach<EnemyDeath, Transform, Velocity>(
			[&](Entity entity, EnemyDeath& death, Transform& transform, Velocity& velocity)
			{
				switch (death.state)
				{
				case EnemyDeath::State::DeathPause:
					velocity.x = 0.0f;
					velocity.y = 0.0f;
					death.stateTimer -= deltaTime;
					if (death.stateTimer <= 0.0f)
					{
						death.state = EnemyDeath::State::DeathFalling;

						// Health = 0 makes PhysicsSystem skip tilemap collision so the
						// entity can pass through the ground after the bounce.
						if (registry.Has<Health>(entity))
							registry.Get<Health>(entity).current = 0;

						// Flying enemies patrol with zero gravity; restore it so the
						// corpse arcs and falls off-screen like grounded enemies do.
						if (registry.Has<Gravity>(entity)
							&& registry.Get<Gravity>(entity).acceleration <= 0.0f)
						{
							registry.Get<Gravity>(entity) = { EnemyDeath::FallGravity, EnemyDeath::MaxFallSpeed };
						}

						velocity.y = -EnemyDeath::DeathBounceSpeed;

						const int   spinDir   = (Random::Int(0, 1) == 0) ? 1 : -1;
						const float spinSpeed = EnemyDeath::SpinBaseSpeed + Random::Float(0.0f, EnemyDeath::SpinSpeedVariance);
						registry.Add<Rotation>(entity, { 0.0f, static_cast<float>(spinDir) * spinSpeed });
					}
					break;

				case EnemyDeath::State::DeathFalling:
					if (registry.Has<Rotation>(entity))
					{
						Rotation& rot = registry.Get<Rotation>(entity);
						rot.angle += rot.spinSpeed * deltaTime;
					}

					if (transform.y > fallLimit)
						toDestroy.push_back(entity);
					break;
				}
			});

		for (const Entity e : toDestroy)
			registry.DestroyEntity(e);
	}
}
