#include "BulletSystem.h"

#include "components/Animation.h"
#include "components/AnimationState.h"
#include "components/Bullet.h"
#include "components/BulletPiece.h"
#include "components/Collider.h"
#include "components/Health.h"
#include "components/Player.h"
#include "components/PreviousTransform.h"
#include "components/Sprite.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <cmath>
#include <vector>

namespace ECS
{
	BulletSystem::BulletSystem(Registry& registry, const Tilemap& tilemap)
		: registry(registry)
		, tilemap(tilemap)
	{}

	void BulletSystem::Update(float deltaTime)
	{
		// Apply gravity to all bullet pieces each frame.
		registry.ForEach<BulletPiece, Velocity>(
			[deltaTime](Entity, BulletPiece& piece, Velocity& velocity)
			{
				velocity.y += piece.gravityAccel * deltaTime;
			});

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

		const float tileSize   = static_cast<float>(tilemap.tileSize);
		const float levelRight = static_cast<float>(tilemap.GetWidth() * tilemap.tileSize);

		struct PieceRequest { float x, y; int direction; };
		std::vector<PieceRequest> pieceRequests;
		std::vector<Entity>       toDestroy;

		registry.ForEach<Bullet, Transform>(
			[&](Entity entity, Bullet& bullet, Transform& transform)
			{
				// Check the leading edge of the bullet against the tilemap.
				const float probeX = transform.x + static_cast<float>(bullet.direction) * 8.0f;
				const float probeY = transform.y - 8.0f; // vertical centre of bullet
				const int   col    = static_cast<int>(std::floor(probeX / tileSize));
				const int   row    = static_cast<int>(std::floor(probeY / tileSize));

				if (tilemap.IsSolid(col, row))
				{
					pieceRequests.push_back({transform.x, transform.y, bullet.direction});
					toDestroy.push_back(entity);
					return;
				}

				// Destroy if it has left the level bounds.
				if (transform.x < -16.0f || transform.x > levelRight + 16.0f)
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

						const float sign = static_cast<float>(bullet.direction);
						playerVelocity->x = sign * playerHealth->knockbackSpeed;
						playerVelocity->y = -playerHealth->knockbackSpeed;
						playerHealth->invulnerabilityTimer = playerHealth->invulnerabilityDuration;
						playerHealth->hitStunTimer         = playerHealth->hitStunDuration;

						if (playerAnimState)
							playerAnimState->current = "Hit";
					}
				}
			});

		// Spawn pieces before destroying bullets so positions are still valid.
		for (const auto& req : pieceRequests)
			SpawnPieces(req.x, req.y, req.direction);

		for (const Entity e : toDestroy)
			registry.DestroyEntity(e);

		// Destroy pieces that have fallen off the bottom of the screen.
		toDestroy.clear();
		registry.ForEach<BulletPiece, Transform>(
			[&](Entity entity, BulletPiece&, Transform& transform)
			{
				if (transform.y > 370.0f)
					toDestroy.push_back(entity);
			});

		for (const Entity e : toDestroy)
			registry.DestroyEntity(e);
	}

	void BulletSystem::SpawnPieces(float x, float y, int bulletDirection)
	{
		for (int i = 0; i < 2; i++)
		{
			const float offsetX = (i == 0) ? -3.0f : 3.0f;
			const float offsetY = (i == 0) ? -6.0f : -2.0f;

			// Pieces bounce in the opposite direction from the bullet's travel.
			const float velX = static_cast<float>(-bulletDirection) * (25.0f + static_cast<float>(i) * 20.0f);
			const float velY = -50.0f - static_cast<float>(i) * 30.0f;

			Entity piece = registry.CreateEntity();
			registry.Add<Transform>(piece, {x + offsetX, y + offsetY});
			registry.Add<PreviousTransform>(piece, {x + offsetX, y + offsetY});
			registry.Add<Velocity>(piece, {velX, velY});

			Sprite sprite;
			sprite.textureName = "trunk_bullet_pieces";
			registry.Add<Sprite>(piece, sprite);

			// No AnimationState — AnimationSystem will leave this alone.
			// Set up Animation manually so the correct piece frame is shown.
			Animation anim;
			anim.data.textureName  = "trunk_bullet_pieces";
			anim.data.frameCount   = 2;
			anim.data.frameDuration = 1.0f;
			anim.currentFrame      = i;
			registry.Add<Animation>(piece, anim);

			registry.Add<BulletPiece>(piece, {i, 400.0f});
		}
	}
}
