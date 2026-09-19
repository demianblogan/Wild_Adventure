#include "TrunkSystem.h"

#include "components/render/Animation.h"
#include "components/render/AnimationState.h"
#include "components/combat/Bullet.h"
#include "components/physics/Collider.h"
#include "components/combat/EnemyDeath.h"
#include "components/physics/Facing.h"
#include "components/ai/GroundPatrol.h"
#include "components/tags/Player.h"
#include "components/physics/PreviousTransform.h"
#include "components/render/Sprite.h"
#include "components/physics/Transform.h"
#include "components/ai/TrunkAI.h"
#include "components/physics/Velocity.h"
#include "core/ecs/Registry.h"

#include <cmath>

namespace ECS
{
	namespace
	{
		constexpr float BulletSpawnSideGap = 6.0f; // clears the trunk's own collider
		constexpr float BulletSpawnRise = 5.0f;     // just above the trunk's base
	}

	TrunkSystem::TrunkSystem(Registry& registry)
		: registry(registry)
	{}

	void TrunkSystem::Update(float deltaTime)
	{
		// Capture the player's position by value, not by reference: SpawnBullet adds
		// components below, which can reallocate a component pool and dangle a held
		// reference into it.
		bool  hasFoundPlayer = false;
		float playerX     = 0.0f;
		float playerCenterY = 0.0f;
		registry.ForEach<Player, Transform, Collider>(
			[&](Entity, Player&, Transform& transform, Collider& collider)
			{
				playerX       = transform.x;
				playerCenterY = transform.y - collider.height * 0.5f;
				hasFoundPlayer   = true;
			});

		if (!hasFoundPlayer)
			return;

		registry.ForEach<TrunkAI, GroundPatrol, Transform, Collider, Velocity, AnimationState, Facing>(
			[&](Entity entity, TrunkAI& trunk, GroundPatrol& patrol,
				Transform& transform, Collider& collider,
				Velocity& velocity, AnimationState& animState, Facing& facing)
			{
				if (registry.Has<EnemyDeath>(entity))
					return;

				const float trunkCenterY  = transform.y - collider.height * 0.5f;
				const float dx            = playerX - transform.x;
				const bool  sameLevel     = std::abs(playerCenterY - trunkCenterY) < TrunkAI::SightTolerance;
				const bool  inRange       = std::abs(dx) < TrunkAI::SightRange;
				const bool  playerInFront = (dx * static_cast<float>(patrol.direction)) > 0.0f;
				const bool  playerVisible = sameLevel && inRange && playerInFront;

				switch (trunk.state)
				{
				case TrunkAI::State::Patrolling:
					patrol.isPaused = false;
					if (playerVisible)
					{
						trunk.state           = TrunkAI::State::Shooting;
						trunk.shootCooldown   = 0.0f;
						trunk.hasFiredBullet  = false;
						patrol.isPaused       = true;
						velocity.x            = 0.0f;
						patrol.direction      = (dx >= 0.0f) ? 1 : -1;
						facing.isLookingRight = (dx >= 0.0f);
						animState.current     = "Attack";
					}
					break;

				case TrunkAI::State::Shooting:
					if (!playerVisible)
					{
						trunk.state          = TrunkAI::State::Patrolling;
						trunk.hasFiredBullet = false;
						patrol.isPaused      = false;
						animState.current    = "Run";
						break;
					}

					// Keep stopped and facing the player even if they strafed slightly.
					patrol.isPaused         = true;
					velocity.x              = 0.0f;
					patrol.direction        = (dx >= 0.0f) ? 1 : -1;
					facing.isLookingRight   = (dx >= 0.0f);

					// Count down cooldown between shots.
					if (trunk.shootCooldown > 0.0f)
					{
						trunk.shootCooldown -= deltaTime;
						if (trunk.shootCooldown <= 0.0f)
						{
							trunk.shootCooldown  = 0.0f;
							trunk.hasFiredBullet = false; // reset so new attack can fire
							animState.current    = "Attack";
						}
						break;
					}

					// Wait for the attack animation; fire on TrunkAI::FireFrame, transition after finish.
					if (registry.Has<Animation>(entity))
					{
						const Animation& anim = registry.Get<Animation>(entity);
						if (anim.playingState == "Attack")
						{
							if (!trunk.hasFiredBullet && anim.currentFrame >= TrunkAI::FireFrame)
							{
								SpawnBullet(transform, collider, patrol.direction);
								trunk.hasFiredBullet = true;
							}
							if (anim.isFinished)
							{
								trunk.shootCooldown  = TrunkAI::ShootInterval;
								trunk.hasFiredBullet = false;
								animState.current    = "Idle";
							}
						}
					}
					break;
				}
			});
	}

	void TrunkSystem::SpawnBullet(const Transform& transform, const Collider& collider, int direction)
	{
		const float spawnX = transform.x + static_cast<float>(direction) * (collider.width / 2.0f + BulletSpawnSideGap);
		const float spawnY = transform.y - BulletSpawnRise;

		Entity bullet = registry.CreateEntity();
		registry.Add<Transform>(bullet, {spawnX, spawnY});
		registry.Add<PreviousTransform>(bullet, {spawnX, spawnY});
		registry.Add<Velocity>(bullet, {TrunkAI::BulletSpeed * static_cast<float>(direction), 0.0f});

		Sprite sprite;
		sprite.textureName = "trunk_bullet";
		registry.Add<Sprite>(bullet, sprite);

		registry.Add<Bullet>(bullet, {direction, 0});
	}
}
