#include "FireSystem.h"

#include "components/render/Animation.h"
#include "components/render/AnimationState.h"
#include "components/physics/Collider.h"
#include "components/traps/Fire.h"
#include "components/combat/Hazard.h"
#include "components/physics/Hitbox.h"
#include "components/physics/Transform.h"
#include "core/ecs/Registry.h"
#include "systems/core/PlayerQuery.h"

namespace ECS
{
	FireSystem::FireSystem(Registry& registry)
		: registry(registry)
	{}

	void FireSystem::Update(float deltaTime)
	{
		// The player only matters for activation; the burn is left to DamageSystem.
		const Entity playerEntity = FindPlayer(registry);

		const Transform* playerTransform = nullptr;
		const Collider*  playerCollider  = nullptr;
		if (playerEntity != INVALID_ENTITY)
		{
			playerTransform = &registry.Get<Transform>(playerEntity);
			playerCollider  = &registry.Get<Collider>(playerEntity);
		}

		registry.ForEach<Fire, Transform, Hitbox, AnimationState>(
			[&](Entity entity, Fire& fire, Transform& transform, Hitbox& hitbox, AnimationState& animState)
			{
				bool playerOnPlate = false;
				if (playerTransform != nullptr)
				{
					const float hHalfW = hitbox.width / 2.0f;
					const float pHalfW = playerCollider->width / 2.0f;
					playerOnPlate =
						(transform.x - hHalfW) < (playerTransform->x + pHalfW) &&
						(transform.x + hHalfW) > (playerTransform->x - pHalfW) &&
						(transform.y - hitbox.height) < playerTransform->y &&
						transform.y > (playerTransform->y - playerCollider->height);
				}

				switch (fire.state)
				{
				case Fire::State::Off:
					if (playerOnPlate)
					{
						fire.state        = Fire::State::Activating;
						animState.current = "Hit";
					}
					break;

				case Fire::State::Activating:
					// Warm-up: once the Hit clip finishes, ignite.
					if (registry.Has<Animation>(entity))
					{
						const Animation& anim = registry.Get<Animation>(entity);
						if (anim.playingState == "Hit" && anim.isFinished)
						{
							fire.state        = Fire::State::On;
							fire.onTimer      = Fire::ON_DURATION;
							animState.current = "On";
							// Becomes harmful: DamageSystem burns anyone overlapping it.
							registry.Add<Hazard>(entity, { Fire::BURN_DAMAGE });
						}
					}
					break;

				case Fire::State::On:
					fire.onTimer -= deltaTime;
					if (fire.onTimer <= 0.0f)
					{
						fire.state        = Fire::State::Off;
						animState.current = "Off";
						if (registry.Has<Hazard>(entity))
							registry.RemoveFrom<Hazard>(entity);
					}
					break;
				}
			});
	}
}
