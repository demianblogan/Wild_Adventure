#include "DeathSystem.h"

#include "components/Health.h"
#include "components/Player.h"
#include "components/Rotation.h"
#include "core/Random.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	DeathSystem::DeathSystem(Registry& registry)
		: registry(registry)
	{}

	void DeathSystem::Update(float deltaTime)
	{
		registry.ForEach<Player, Health>(
			[this, deltaTime](Entity entity, Player&, Health& health)
			{
				if (health.current > 0)
					return;

				// First frame of death: start a tumble in a random direction.
				if (!registry.Has<Rotation>(entity))
				{
					const float sign = (Random::Int(0, 1) == 0) ? -1.0f : 1.0f;

					Rotation rotation;
					rotation.spinSpeed = sign * Random::Float(360.0f, 540.0f);
					registry.Add<Rotation>(entity, rotation);
				}

				registry.Get<Rotation>(entity).angle += registry.Get<Rotation>(entity).spinSpeed * deltaTime;
			});
	}
}