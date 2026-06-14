#include "BeeSystem.h"

#include "components/Animation.h"
#include "components/AnimationState.h"
#include "components/BeeAI.h"
#include "components/Bullet.h"
#include "components/Collider.h"
#include "components/EnemyDeath.h"
#include "components/Player.h"
#include "components/PreviousTransform.h"
#include "components/Sprite.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <cmath>

namespace ECS
{
	BeeSystem::BeeSystem(Registry& registry)
		: registry(registry)
	{}

	void BeeSystem::Update(float deltaTime)
	{
		// Capture the player's X by value, not by reference: SpawnBullet adds components
		// below, which can reallocate the Transform pool and dangle a held pointer.
		bool  playerFound = false;
		float playerX     = 0.0f;
		registry.ForEach<Player, Transform>(
			[&](Entity, Player&, Transform& transform) { playerX = transform.x; playerFound = true; });

		registry.ForEach<BeeAI, Transform, Collider, Velocity, AnimationState>(
			[&](Entity entity, BeeAI& bee, Transform& transform, Collider& collider,
				Velocity& velocity, AnimationState& animState)
			{
				// EnemyDeathSystem owns the bee once it starts dying.
				if (registry.Has<EnemyDeath>(entity))
					return;

				// Capture the spawn point as the centre of the figure-8 on first tick.
				if (!bee.initialized)
				{
					bee.anchorX     = transform.x;
					bee.anchorY     = transform.y;
					bee.initialized = true;
				}

				// Drive the figure-8 by steering velocity toward the exact path point for
				// the current time. Computing the target from absolute time (rather than
				// integrating) keeps the path drift-free. PhysicsSystem then moves the bee
				// and owns PreviousTransform, so render interpolation stays correct.
				bee.time += deltaTime;
				const float angle   = bee.time * bee.speed;
				const float targetX = bee.anchorX + (bee.width  * 0.5f) * std::sin(angle);
				const float targetY = bee.anchorY + (bee.height * 0.5f) * std::sin(2.0f * angle);

				if (deltaTime > 0.0f)
				{
					velocity.x = (targetX - transform.x) / deltaTime;
					velocity.y = (targetY - transform.y) / deltaTime;
				}

				// The player is "near" purely by horizontal distance: the bee rains bullets
				// straight down, so vertical alignment does not matter.
				const float attackRange = collider.width * BeeAI::ATTACK_RANGE_FACTOR;
				const bool  playerNear  = playerFound
					&& std::abs(playerX - transform.x) < attackRange;

				switch (bee.state)
				{
				case BeeAI::State::Flying:
					if (playerNear)
					{
						bee.state         = BeeAI::State::Attacking;
						bee.shootCooldown = 0.0f;
						bee.bulletFired   = false;
						animState.current = "Attack";
					}
					break;

				case BeeAI::State::Attacking:
					if (!playerNear)
					{
						bee.state         = BeeAI::State::Flying;
						bee.bulletFired   = false;
						animState.current = "Idle";
						break;
					}

					// Count down the gap between shots; restart the attack when it elapses.
					if (bee.shootCooldown > 0.0f)
					{
						bee.shootCooldown -= deltaTime;
						if (bee.shootCooldown <= 0.0f)
						{
							bee.shootCooldown = 0.0f;
							bee.bulletFired   = false;
							animState.current = "Attack";
						}
						break;
					}

					// Fire mid-animation, then drop back to Idle once it finishes.
					if (registry.Has<Animation>(entity))
					{
						const Animation& anim = registry.Get<Animation>(entity);
						if (anim.playingState == "Attack")
						{
							if (!bee.bulletFired && anim.currentFrame >= BeeAI::FIRE_FRAME)
							{
								SpawnBullet(transform);
								bee.bulletFired = true;
							}
							if (anim.isFinished)
							{
								bee.shootCooldown = BeeAI::SHOOT_INTERVAL;
								bee.bulletFired   = false;
								animState.current = "Idle";
							}
						}
					}
					break;
				}
			});
	}

	void BeeSystem::SpawnBullet(const Transform& transform)
	{
		const float spawnX = transform.x;
		const float spawnY = transform.y + 4.0f; // just below the bee's body

		Entity bullet = registry.CreateEntity();
		registry.Add<Transform>(bullet, {spawnX, spawnY});
		registry.Add<PreviousTransform>(bullet, {spawnX, spawnY});
		registry.Add<Velocity>(bullet, {0.0f, BeeAI::BULLET_SPEED});

		Sprite sprite;
		sprite.textureName = "bee_bullet";
		registry.Add<Sprite>(bullet, sprite);

		Bullet data;
		data.dirX          = 0;
		data.dirY          = 1;
		data.piecesTexture = "bee_bullet_pieces";
		registry.Add<Bullet>(bullet, data);
	}
}
