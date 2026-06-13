#include "PlantSystem.h"

#include "components/Animation.h"
#include "components/AnimationState.h"
#include "components/Bullet.h"
#include "components/Collider.h"
#include "components/EnemyDeath.h"
#include "components/Facing.h"
#include "components/PlantAI.h"
#include "components/Player.h"
#include "components/PreviousTransform.h"
#include "components/Sprite.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <cmath>

namespace ECS
{
	PlantSystem::PlantSystem(Registry& registry)
		: registry(registry)
	{}

	void PlantSystem::Update(float deltaTime)
	{
		Entity playerEntity = INVALID_ENTITY;
		registry.ForEach<Player>([&](Entity entity, Player&) { playerEntity = entity; });

		if (playerEntity == INVALID_ENTITY)
			return;

		const Transform& playerTransform = registry.Get<Transform>(playerEntity);
		const Collider&  playerCollider  = registry.Get<Collider>(playerEntity);
		const float      playerCenterY   = playerTransform.y - playerCollider.height * 0.5f;

		registry.ForEach<PlantAI, Transform, Collider, AnimationState, Facing>(
			[&](Entity entity, PlantAI& plant, Transform& transform, Collider& collider,
				AnimationState& animState, Facing& facing)
			{
				if (registry.Has<EnemyDeath>(entity))
					return;

				// The plant cannot turn; it only sees the player on the side it faces.
				const int   facingDir     = facing.isLookingRight ? 1 : -1;
				const float plantCenterY   = transform.y - collider.height * 0.5f;
				const float dx             = playerTransform.x - transform.x;
				const bool  sameLevel      = std::abs(playerCenterY - plantCenterY) < PlantAI::SIGHT_TOLERANCE;
				const bool  inRange        = std::abs(dx) < PlantAI::SIGHT_RANGE;
				const bool  playerInFront  = (dx * static_cast<float>(facingDir)) > 0.0f;
				const bool  playerVisible  = sameLevel && inRange && playerInFront;

				switch (plant.state)
				{
				case PlantAI::State::Idle:
					if (playerVisible)
					{
						plant.state         = PlantAI::State::Attacking;
						plant.shootCooldown = 0.0f;
						plant.bulletFired   = false;
						animState.current   = "Attack";
					}
					break;

				case PlantAI::State::Attacking:
					if (!playerVisible)
					{
						plant.state       = PlantAI::State::Idle;
						plant.bulletFired = false;
						animState.current = "Idle";
						break;
					}

					// Count down the gap between shots; restart the attack when it elapses.
					if (plant.shootCooldown > 0.0f)
					{
						plant.shootCooldown -= deltaTime;
						if (plant.shootCooldown <= 0.0f)
						{
							plant.shootCooldown = 0.0f;
							plant.bulletFired   = false;
							animState.current   = "Attack";
						}
						break;
					}

					// Fire mid-animation, then drop back to Idle once it finishes.
					if (registry.Has<Animation>(entity))
					{
						const Animation& anim = registry.Get<Animation>(entity);
						if (anim.playingState == "Attack")
						{
							if (!plant.bulletFired && anim.currentFrame >= PlantAI::FIRE_FRAME)
							{
								SpawnBullet(transform, collider, facingDir);
								plant.bulletFired = true;
							}
							if (anim.isFinished)
							{
								plant.shootCooldown = PlantAI::SHOOT_INTERVAL;
								plant.bulletFired   = false;
								animState.current   = "Idle";
							}
						}
					}
					break;
				}
			});
	}

	void PlantSystem::SpawnBullet(const Transform& transform, const Collider& collider, int direction)
	{
		const float spawnX = transform.x + static_cast<float>(direction) * (collider.width / 2.0f + 6.0f);
		// Bullet exits the plant's mouth, a fixed height above its base.
		const float spawnY = transform.y - PlantAI::BULLET_HEIGHT;

		Entity bullet = registry.CreateEntity();
		registry.Add<Transform>(bullet, {spawnX, spawnY});
		registry.Add<PreviousTransform>(bullet, {spawnX, spawnY});
		registry.Add<Velocity>(bullet, {PlantAI::BULLET_SPEED * static_cast<float>(direction), 0.0f});

		Sprite sprite;
		sprite.textureName = "plant_bullet";
		registry.Add<Sprite>(bullet, sprite);

		Bullet data;
		data.direction     = direction;
		data.piecesTexture = "plant_bullet_pieces";
		registry.Add<Bullet>(bullet, data);
	}
}
