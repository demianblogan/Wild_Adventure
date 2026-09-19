#include "DeathSystem.h"

#include "components/combat/Health.h"
#include "components/tags/Player.h"
#include "components/physics/Rotation.h"
#include "core/Random.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	namespace
	{
		constexpr float TumbleSpinMin = 360.0f; // degrees/second
		constexpr float TumbleSpinMax = 540.0f;
	}

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
					rotation.spinSpeed = sign * Random::Float(TumbleSpinMin, TumbleSpinMax);
					registry.Add<Rotation>(entity, rotation);
				}

				registry.Get<Rotation>(entity).angle += registry.Get<Rotation>(entity).spinSpeed * deltaTime;
			});
	}
}