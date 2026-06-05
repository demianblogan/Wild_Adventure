#include "DeathSystem.h"

#include "components/Health.h"
#include "components/Player.h"
#include "components/Rotation.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	DeathSystem::DeathSystem(Registry& registry)
		: registry(registry)
		, randomEngine(std::random_device{}())
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
					std::uniform_int_distribution<int> sideDistribution(0, 1);
					std::uniform_real_distribution<float> speedDistribution(360.0f, 540.0f);

					const float sign = (sideDistribution(randomEngine) == 0) ? -1.0f : 1.0f;

					Rotation rotation;
					rotation.spinSpeed = sign * speedDistribution(randomEngine);
					registry.Add<Rotation>(entity, rotation);
				}

				registry.Get<Rotation>(entity).angle += registry.Get<Rotation>(entity).spinSpeed * deltaTime;
			});
	}
}