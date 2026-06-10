#include "TrunkSystem.h"

#include "components/Animation.h"
#include "components/AnimationState.h"
#include "components/Bullet.h"
#include "components/Collider.h"
#include "components/EnemyDeath.h"
#include "components/Facing.h"
#include "components/GroundPatrol.h"
#include "components/Player.h"
#include "components/PreviousTransform.h"
#include "components/Sprite.h"
#include "components/Transform.h"
#include "components/TrunkAI.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <cmath>

namespace ECS
{
	TrunkSystem::TrunkSystem(Registry& registry)
		: registry(registry)
	{}

	void TrunkSystem::Update(float deltaTime)
	{
		Entity playerEntity = INVALID_ENTITY;
		registry.ForEach<Player>([&](Entity entity, Player&) { playerEntity = entity; });

		if (playerEntity == INVALID_ENTITY)
			return;

		const Transform& playerTransform = registry.Get<Transform>(playerEntity);
		const Collider&  playerCollider  = registry.Get<Collider>(playerEntity);
		const float      playerCenterY   = playerTransform.y - playerCollider.height * 0.5f;

		registry.ForEach<TrunkAI, GroundPatrol, Transform, Collider, Velocity, AnimationState, Facing>(
			[&](Entity entity, TrunkAI& trunk, GroundPatrol& patrol,
				Transform& transform, Collider& collider,
				Velocity& velocity, AnimationState& animState, Facing& facing)
			{
				if (registry.Has<EnemyDeath>(entity))
					return;

				const float trunkCenterY  = transform.y - collider.height * 0.5f;
				const float dx            = playerTransform.x - transform.x;
				const bool  sameLevel     = std::abs(playerCenterY - trunkCenterY) < TrunkAI::SIGHT_TOLERANCE;
				const bool  inRange       = std::abs(dx) < TrunkAI::SIGHT_RANGE;
				const bool  playerInFront = (dx * static_cast<float>(patrol.direction)) > 0.0f;
				const bool  playerVisible = sameLevel && inRange && playerInFront;

				switch (trunk.state)
				{
				case TrunkAI::State::Patrolling:
					patrol.paused = false;
					if (playerVisible)
					{
						trunk.state         = TrunkAI::State::Shooting;
						trunk.shootCooldown = 0.0f;
						trunk.bulletFired   = false;
						patrol.paused       = true;
						velocity.x          = 0.0f;
						patrol.direction    = (dx >= 0.0f) ? 1 : -1;
						facing.isLookingRight = (dx >= 0.0f);
						animState.current   = "Attack";
					}
					break;

				case TrunkAI::State::Shooting:
					if (!playerVisible)
					{
						trunk.state       = TrunkAI::State::Patrolling;
						trunk.bulletFired = false;
						patrol.paused     = false;
						animState.current = "Run";
						break;
					}

					// Keep stopped and facing the player even if they strafed slightly.
					patrol.paused         = true;
					velocity.x            = 0.0f;
					patrol.direction      = (dx >= 0.0f) ? 1 : -1;
					facing.isLookingRight = (dx >= 0.0f);

					// Count down cooldown between shots.
					if (trunk.shootCooldown > 0.0f)
					{
						trunk.shootCooldown -= deltaTime;
						if (trunk.shootCooldown <= 0.0f)
						{
							trunk.shootCooldown   = 0.0f;
							trunk.bulletFired     = false; // reset so new attack can fire
							animState.current     = "Attack";
						}
						break;
					}

					// Wait for the attack animation; fire on frame 8, transition after finish.
					if (registry.Has<Animation>(entity))
					{
						const Animation& anim = registry.Get<Animation>(entity);
						if (anim.playingState == "Attack")
						{
							if (!trunk.bulletFired && anim.currentFrame >= 7)
							{
								SpawnBullet(transform, collider, patrol.direction);
								trunk.bulletFired = true;
							}
							if (anim.isFinished)
							{
								trunk.shootCooldown = TrunkAI::SHOOT_INTERVAL;
								trunk.bulletFired   = false;
								animState.current   = "Idle";
							}
						}
					}
					break;
				}
			});
	}

	void TrunkSystem::SpawnBullet(const Transform& transform, const Collider& collider, int direction)
	{
		const float spawnX = transform.x + static_cast<float>(direction) * (collider.width / 2.0f + 6.0f);
		const float spawnY = transform.y - 5.0f;

		Entity bullet = registry.CreateEntity();
		registry.Add<Transform>(bullet, {spawnX, spawnY});
		registry.Add<PreviousTransform>(bullet, {spawnX, spawnY});
		registry.Add<Velocity>(bullet, {TrunkAI::BULLET_SPEED * static_cast<float>(direction), 0.0f});

		Sprite sprite;
		sprite.textureName = "trunk_bullet";
		registry.Add<Sprite>(bullet, sprite);

		registry.Add<Bullet>(bullet, {direction});
	}
}
