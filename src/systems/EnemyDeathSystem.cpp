#include "EnemyDeathSystem.h"

#include "components/EnemyDeath.h"
#include "components/Health.h"
#include "components/Rotation.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <cstdlib>
#include <vector>

namespace ECS
{
	EnemyDeathSystem::EnemyDeathSystem(Registry& registry)
		: registry(registry)
	{}

	void EnemyDeathSystem::Update(float deltaTime)
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

						velocity.y = -EnemyDeath::DEATH_BOUNCE_SPEED;

						const int   spinDir   = (std::rand() % 2 == 0) ? 1 : -1;
						const float spinSpeed = 270.0f + static_cast<float>(std::rand() % 90);
						registry.Add<Rotation>(entity, { 0.0f, static_cast<float>(spinDir) * spinSpeed });
					}
					break;

				case EnemyDeath::State::DeathFalling:
					if (registry.Has<Rotation>(entity))
					{
						Rotation& rot = registry.Get<Rotation>(entity);
						rot.angle += rot.spinSpeed * deltaTime;
					}

					if (transform.y > 370.0f)
						toDestroy.push_back(entity);
					break;
				}
			});

		for (const Entity e : toDestroy)
			registry.DestroyEntity(e);
	}
}
