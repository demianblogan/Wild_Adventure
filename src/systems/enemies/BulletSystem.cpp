#include "BulletSystem.h"

#include "components/AnimationState.h"
#include "components/Bullet.h"
#include "components/Collider.h"
#include "components/Health.h"
#include "components/Player.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"
#include "graphics/ParticleSystem.h"

#include <cmath>
#include <vector>

namespace ECS
{
	BulletSystem::BulletSystem(Registry& registry, const Tilemap& tilemap, ParticleSystem& particles)
		: registry(registry)
		, tilemap(tilemap)
		, particles(particles)
	{}

	void BulletSystem::Update(float deltaTime)
	{
		// Locate the player for collision checks.
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

		const float tileSize    = static_cast<float>(tilemap.tileSize);
		const float levelRight  = static_cast<float>(tilemap.GetWidth() * tilemap.tileSize);
		const float levelBottom = static_cast<float>(tilemap.GetHeight() * tilemap.tileSize);

		struct PieceRequest { float x, y; int bounceDir; std::string piecesTexture; };
		std::vector<PieceRequest> pieceRequests;
		std::vector<Entity>       toDestroy;

		registry.ForEach<Bullet, Transform>(
			[&](Entity entity, Bullet& bullet, Transform& transform)
			{
				// Probe the leading edge along the bullet's travel direction. The bullet
				// sprite is 16x16 with a bottom-centre origin, so its centre sits 8px up.
				const float centerY = transform.y - 8.0f;
				const float probeX  = transform.x + static_cast<float>(bullet.dirX) * 8.0f;
				const float probeY  = centerY     + static_cast<float>(bullet.dirY) * 8.0f;
				const int   col     = static_cast<int>(std::floor(probeX / tileSize));
				const int   row     = static_cast<int>(std::floor(probeY / tileSize));

				if (tilemap.IsSolid(col, row))
				{
					// Horizontal bullets bounce their pieces back off the wall; a straight-
					// down bullet (dirX == 0) lets them spread both ways like box debris.
					pieceRequests.push_back({transform.x, transform.y, -bullet.dirX, bullet.piecesTexture});
					toDestroy.push_back(entity);
					return;
				}

				// Destroy if it has left the level bounds.
				if (transform.x < -16.0f || transform.x > levelRight + 16.0f
					|| transform.y < -16.0f || transform.y > levelBottom + 16.0f)
				{
					toDestroy.push_back(entity);
					return;
				}

				// Player overlap.
				if (playerTransform
					&& playerHealth->current > 0
					&& playerHealth->invulnerabilityTimer <= 0.0f)
				{
					const float pHalfW = playerCollider->width / 2.0f;
					const bool  overlap =
						(transform.x - 8.0f) < (playerTransform->x + pHalfW) &&
						(transform.x + 8.0f) > (playerTransform->x - pHalfW) &&
						(transform.y - 16.0f) < playerTransform->y &&
						transform.y           > (playerTransform->y - playerCollider->height);

					if (overlap)
					{
						toDestroy.push_back(entity);

						playerHealth->current -= 1;

						const float sign = static_cast<float>(bullet.dirX);
						playerVelocity->x = sign * playerHealth->knockbackSpeed;
						playerVelocity->y = -playerHealth->knockbackSpeed;
						playerHealth->invulnerabilityTimer = playerHealth->invulnerabilityDuration;
						playerHealth->hitStunTimer         = playerHealth->hitStunDuration;

						if (playerAnimState)
							playerAnimState->current = "Hit";
					}
				}
			});

		// Spawn debris before destroying bullets so positions are still valid.
		// Pieces bounce back off the wall (opposite the bullet's travel direction) and
		// then fall, rest, and blink out exactly like box debris.
		for (const auto& req : pieceRequests)
			particles.EmitDebris({req.x, req.y - 8.0f}, req.piecesTexture, 2, req.bounceDir);

		for (const Entity e : toDestroy)
			registry.DestroyEntity(e);
	}
}
