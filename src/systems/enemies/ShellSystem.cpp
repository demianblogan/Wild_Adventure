#include "ShellSystem.h"

#include "audio/Mixer.h"
#include "components/Animation.h"
#include "components/AnimationState.h"
#include "components/Collider.h"
#include "components/CollisionState.h"
#include "components/EnemyDeath.h"
#include "components/Facing.h"
#include "components/Health.h"
#include "components/Jump.h"
#include "components/Player.h"
#include "components/Shell.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	ShellSystem::ShellSystem(Registry& registry, Audio::Mixer& mixer)
		: registry(registry)
		, mixer(mixer)
	{}

	void ShellSystem::Update(float deltaTime)
	{
		Entity playerEntity = INVALID_ENTITY;
		registry.ForEach<Player>([&](Entity entity, Player&) { playerEntity = entity; });

		Transform*      playerTransform = nullptr;
		Collider*       playerCollider  = nullptr;
		Health*         playerHealth    = nullptr;
		Velocity*       playerVelocity  = nullptr;
		AnimationState* playerAnimState = nullptr;

		if (playerEntity != INVALID_ENTITY)
		{
			playerTransform = &registry.Get<Transform>(playerEntity);
			playerCollider  = &registry.Get<Collider>(playerEntity);
			playerHealth    = &registry.Get<Health>(playerEntity);
			playerVelocity  = &registry.Get<Velocity>(playerEntity);
			if (registry.Has<AnimationState>(playerEntity))
				playerAnimState = &registry.Get<AnimationState>(playerEntity);
		}

		// ShellSystem never spawns entities, so holding player pointers across the loop
		// is safe (no component pool is grown while iterating).
		registry.ForEach<Shell, Transform, Collider, Velocity, AnimationState, CollisionState>(
			[&](Entity entity, Shell& shell, Transform& transform, Collider& collider,
				Velocity& velocity, AnimationState& animState, CollisionState& collisionState)
			{
				if (registry.Has<EnemyDeath>(entity))
					return;

				if (shell.kickGrace > 0.0f)
					shell.kickGrace -= deltaTime;

				// Player overlap (AABB); both origins are bottom-centre.
				bool overlap        = false;
				bool playerHittable = false;
				if (playerTransform && playerHealth->current > 0)
				{
					const float mHalfW = collider.width / 2.0f;
					const float pHalfW = playerCollider->width / 2.0f;
					overlap =
						(transform.x - mHalfW) < (playerTransform->x + pHalfW) &&
						(transform.x + mHalfW) > (playerTransform->x - pHalfW) &&
						(transform.y - collider.height) < playerTransform->y &&
						transform.y > (playerTransform->y - playerCollider->height);
					playerHittable = playerHealth->invulnerabilityTimer <= 0.0f;
				}

				switch (shell.state)
				{
				case Shell::State::Resting:
					velocity.x = 0.0f;
					if (overlap && shell.kickGrace <= 0.0f && playerHittable)
					{
						// Roll away from the player at once (touched on its right -> rolls
						// left); the impact flash plays while it already moves.
						shell.direction        = (playerTransform->x > transform.x) ? -1 : 1;
						shell.state            = Shell::State::Rolling;
						shell.harmlessToKicker = true;
						velocity.x             = shell.speed * static_cast<float>(shell.direction);
						animState.current      = "WallHit";
					}
					break;

				case Shell::State::Rolling:
					velocity.x = shell.speed * static_cast<float>(shell.direction);

					if (collisionState.isOnWall && collisionState.wallDirection == shell.direction)
					{
						// Reverse off the wall without stopping; the flash plays on the move.
						shell.direction   = -shell.direction;
						velocity.x        = shell.speed * static_cast<float>(shell.direction);
						animState.current = "WallHit";
					}
					else if (registry.Has<Animation>(entity))
					{
						// Once the impact flash finishes, fall back to the rolling loop.
						const Animation& anim = registry.Get<Animation>(entity);
						if (anim.playingState == "WallHit" && anim.isFinished)
							animState.current = "Roll";
					}

					// The shell becomes dangerous to the kicker only once they step clear.
					if (!overlap)
						shell.harmlessToKicker = false;

					if (overlap && playerHittable)
					{
						const float playerCenterY = playerTransform->y - playerCollider->height / 2.0f;
						const float shellCenterY  = transform.y - collider.height / 2.0f;
						const bool  isStomping    = playerVelocity->y > 0.0f && playerCenterY < shellCenterY;

						if (isStomping)
						{
							// Stomped: the shell stops and dies like other enemies.
							animState.current = "TopHit";
							registry.Add<EnemyDeath>(entity, {});
							velocity.x = 0.0f;

							mixer.PlaySound("jump_on_enemy");
							playerVelocity->y = -200.0f;
							if (registry.Has<Jump>(playerEntity)
								&& registry.Get<Jump>(playerEntity).jumpsRemaining < 1)
								registry.Get<Jump>(playerEntity).jumpsRemaining = 1;
						}
						else if (!shell.harmlessToKicker)
						{
							// Side contact hurts the player; the shell keeps rolling.
							playerHealth->current -= 1;

							const float sign = (playerTransform->x >= transform.x) ? 1.0f : -1.0f;
							playerVelocity->x = sign * playerHealth->knockbackSpeed;
							playerVelocity->y = -playerHealth->knockbackSpeed;
							playerHealth->invulnerabilityTimer = playerHealth->invulnerabilityDuration;
							playerHealth->hitStunTimer         = playerHealth->hitStunDuration;

							if (playerAnimState)
								playerAnimState->current = "Hit";
						}
					}
					break;
				}

				if (registry.Has<Facing>(entity))
					registry.Get<Facing>(entity).isLookingRight = (shell.direction > 0);
			});
	}
}
