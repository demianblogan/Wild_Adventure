#include "ChickenSystem.h"

#include "components/AnimationState.h"
#include "components/ChickenAI.h"
#include "components/Collider.h"
#include "components/CollisionState.h"
#include "components/EnemyDeath.h"
#include "components/Facing.h"
#include "components/Health.h"
#include "components/Player.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"
#include "graphics/ParticleSystem.h"

#include <cmath>

namespace ECS
{
	ChickenSystem::ChickenSystem(Registry& registry, ParticleSystem& particles)
		: registry(registry)
		, particles(particles)
	{}

	void ChickenSystem::Update(float deltaTime)
	{
		Entity playerEntity = INVALID_ENTITY;
		registry.ForEach<Player>([&](Entity entity, Player&) { playerEntity = entity; });

		if (playerEntity == INVALID_ENTITY)
			return;

		const Transform& playerTransform = registry.Get<Transform>(playerEntity);
		const Collider&  playerCollider  = registry.Get<Collider>(playerEntity);
		const bool       playerAlive     = registry.Get<Health>(playerEntity).current > 0;
		const float      playerCenterY   = playerTransform.y - playerCollider.height * 0.5f;

		registry.ForEach<ChickenAI, Transform, Collider, Velocity, AnimationState, Facing>(
			[&](Entity entity, ChickenAI& chicken, Transform& transform, Collider& collider,
				Velocity& velocity, AnimationState& animState, Facing& facing)
			{
				// EnemyDeathSystem owns the entity once it starts dying.
				if (registry.Has<EnemyDeath>(entity))
					return;

				const float dx             = playerTransform.x - transform.x;
				const float chickenCenterY = transform.y - collider.height * 0.5f;
				const bool  playerVisible  = playerAlive
					&& std::abs(dx) <= chicken.visionWidth * 0.5f
					&& std::abs(playerCenterY - chickenCenterY) <= chicken.visionHeight * 0.5f;

				switch (chicken.state)
				{
				case ChickenAI::State::Idle:
					velocity.x = 0.0f;
					if (playerVisible)
					{
						chicken.state          = ChickenAI::State::Chasing;
						chicken.loseSightTimer = ChickenAI::LOSE_SIGHT_DELAY;
					}
					break;

				case ChickenAI::State::Chasing:
					if (playerVisible)
						chicken.loseSightTimer = ChickenAI::LOSE_SIGHT_DELAY;
					else
						chicken.loseSightTimer -= deltaTime;

					if (chicken.loseSightTimer <= 0.0f)
					{
						chicken.state = ChickenAI::State::Idle;
						velocity.x    = 0.0f;
						break;
					}

					// Dead zone keeps the chicken from jittering right under the player.
					if (std::abs(dx) > ChickenAI::STOP_DISTANCE)
					{
						velocity.x            = (dx > 0.0f ? 1.0f : -1.0f) * chicken.speed;
						facing.isLookingRight = (dx > 0.0f);
					}
					else
					{
						velocity.x = 0.0f;
					}
					break;
				}

				animState.current = (velocity.x != 0.0f) ? "Run" : "Idle";

				// Same run dust as the ground patrollers, only while actually running.
				const bool onGround = registry.Has<CollisionState>(entity)
					&& registry.Get<CollisionState>(entity).isOnGround;

				if (velocity.x != 0.0f && onGround)
				{
					chicken.dustTimer -= deltaTime;
					if (chicken.dustTimer <= 0.0f)
					{
						const float direction = (velocity.x > 0.0f) ? 1.0f : -1.0f;
						const float backX     = transform.x - direction * particles.GetRunBackOffset();
						particles.Emit("run", { backX, transform.y });
						chicken.dustTimer = ChickenAI::DUST_SPACING / chicken.speed;
					}
				}
			});
	}
}
