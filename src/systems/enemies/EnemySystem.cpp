#include "EnemySystem.h"

#include "components/AnimationState.h"
#include "components/Collider.h"
#include "components/Enemy.h"
#include "components/EnemyDeath.h"
#include "components/Health.h"
#include "components/Hidden.h"
#include "components/Jump.h"
#include "components/Player.h"
#include "components/Spiky.h"
#include "components/StompCustomDeath.h"
#include "components/Stomped.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <cmath>

namespace ECS
{
	EnemySystem::EnemySystem(Registry& registry, int& score, Audio::Mixer& mixer, int& enemiesKilled)
		: registry(registry)
		, score(score)
		, mixer(mixer)
		, enemiesKilled(enemiesKilled)
	{}

	void EnemySystem::Update()
	{
		Entity playerEntity = INVALID_ENTITY;
		registry.ForEach<Player>([&](Entity entity, Player&) { playerEntity = entity; });

		if (playerEntity == INVALID_ENTITY)
			return;

		Transform&      playerTransform = registry.Get<Transform>(playerEntity);
		Collider&       playerCollider  = registry.Get<Collider>(playerEntity);
		Health&         playerHealth    = registry.Get<Health>(playerEntity);
		Velocity&       playerVelocity  = registry.Get<Velocity>(playerEntity);

		// Only check interaction if the player is alive and not currently invulnerable.
		if (playerHealth.current <= 0 || playerHealth.invulnerabilityTimer > 0.0f)
			return;

		AnimationState* playerAnimState = nullptr;
		if (registry.Has<AnimationState>(playerEntity))
			playerAnimState = &registry.Get<AnimationState>(playerEntity);

		registry.ForEach<Enemy, Transform, Collider, Health>(
			[&](Entity entity, Enemy& enemy, Transform& transform, Collider& collider, Health& health)
			{
				// Skip enemies that are already dying (or mid custom-death sequence).
				if (health.current <= 0 || registry.Has<EnemyDeath>(entity)
					|| registry.Has<Stomped>(entity) || registry.Has<Hidden>(entity))
					return;

				const float mHalfW = collider.width  / 2.0f;
				const float pHalfW = playerCollider.width / 2.0f;

				const bool overlap =
					(transform.x - mHalfW) < (playerTransform.x + pHalfW) &&
					(transform.x + mHalfW) > (playerTransform.x - pHalfW) &&
					(transform.y - collider.height) < playerTransform.y &&
					transform.y > (playerTransform.y - playerCollider.height);

				if (!overlap)
					return;

				const float playerCenterY = playerTransform.y - playerCollider.height / 2.0f;
				const float enemyCenterY  = transform.y - collider.height / 2.0f;
				const bool  isStomping    = playerVelocity.y > 0.0f && playerCenterY < enemyCenterY;

				// A Spiky enemy (e.g. the turtle with its spikes out) cannot be stomped:
				// a stomp on it counts as ordinary contact and hurts the player instead.
				if (isStomping && !registry.Has<Spiky>(entity))
				{
					// Most enemies die outright; an enemy that opts into a custom death
					// (e.g. the snail) is only tagged, and its own system takes over.
					if (registry.Has<StompCustomDeath>(entity))
						registry.Add<Stomped>(entity, {});
					else
						registry.Add<EnemyDeath>(entity, {});

					if (registry.Has<AnimationState>(entity))
						registry.Get<AnimationState>(entity).current = "Hit";

					mixer.PlaySound("jump_on_enemy");
					playerVelocity.y = -200.0f;
					score += enemy.scoreValue;
					enemiesKilled++;

					// The stomp bounce acts as a first jump: guarantee one air jump,
					// which then plays the double-jump sound and animation.
					if (registry.Has<Jump>(playerEntity)
						&& registry.Get<Jump>(playerEntity).jumpsRemaining < 1)
					{
						registry.Get<Jump>(playerEntity).jumpsRemaining = 1;
					}
				}
				else
				{
					playerHealth.current -= 1;

					const float dx = playerTransform.x - transform.x;
					const float dy = playerCenterY - enemyCenterY;

					if (std::abs(dx) > std::abs(dy))
					{
						const float sign = (dx > 0.0f) ? 1.0f : -1.0f;
						playerVelocity.x = sign * playerHealth.knockbackSpeed;
						playerVelocity.y = -playerHealth.knockbackSpeed;
					}
					else if (dy < 0.0f)
					{
						playerVelocity.x = 0.0f;
						playerVelocity.y = -playerHealth.knockbackSpeed;
					}
					else
					{
						playerVelocity.x = 0.0f;
						playerVelocity.y = playerHealth.knockbackSpeed;
					}

					playerHealth.invulnerabilityTimer = playerHealth.invulnerabilityDuration;
					playerHealth.hitStunTimer         = playerHealth.hitStunDuration;

					if (playerAnimState)
						playerAnimState->current = "Hit";
				}
			});
	}
}
