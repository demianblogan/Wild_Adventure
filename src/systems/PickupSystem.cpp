#include "PickupSystem.h"

#include "components/Animation.h"
#include "components/AnimationState.h"
#include "components/Collectible.h"
#include "components/Collider.h"
#include "components/Despawning.h"
#include "components/Hitbox.h"
#include "components/Player.h"
#include "components/Transform.h"
#include "core/ecs/Registry.h"

#include <vector>

namespace ECS
{
	PickupSystem::PickupSystem(Registry& registry, int& score)
		: registry(registry)
		, score(score)
	{}

	void PickupSystem::Update()
	{
		// 1. Player box.
		bool hasPlayer = false;
		float playerLeft = 0.0f, playerRight = 0.0f, playerTop = 0.0f, playerBottom = 0.0f;

		registry.ForEach<Player, Transform, Collider>(
			[&](Entity, Player&, Transform& transform, Collider& collider)
			{
				const float halfWidth = collider.width / 2.0f;
				playerLeft = transform.x - halfWidth;
				playerRight = transform.x + halfWidth;
				playerTop = transform.y - collider.height;
				playerBottom = transform.y;
				hasPlayer = true;
			});

		if (!hasPlayer)
			return;

		// 2. Find touched fruits (don't mutate during the loop).
		std::vector<Entity> collected;

		registry.ForEach<Collectible, Hitbox, Transform>(
			[&](Entity entity, Collectible&, Hitbox& hitbox, Transform& transform)
			{
				const float halfWidth = hitbox.width / 2.0f;
				const float left = transform.x - halfWidth;
				const float right = transform.x + halfWidth;
				const float top = transform.y - hitbox.height;
				const float bottom = transform.y;

				const bool overlap = playerLeft < right && playerRight > left
					&& playerTop < bottom && playerBottom > top;

				if (overlap)
					collected.push_back(entity);
			});

		for (const Entity entity : collected)
		{
			score += registry.Get<Collectible>(entity).points;

			registry.RemoveFrom<Collectible>(entity);
			if (registry.Has<Hitbox>(entity))
				registry.RemoveFrom<Hitbox>(entity);

			if (registry.Has<AnimationState>(entity))
				registry.Get<AnimationState>(entity).current = "Collected";

			registry.Add<Despawning>(entity, {});
		}

		// 3. Remove fruits whose collect animation has finished.
		std::vector<Entity> finished;

		registry.ForEach<Despawning, Animation>(
			[&](Entity entity, Despawning&, Animation& animation)
			{
				if (!animation.data.isLooping && animation.currentFrame >= animation.data.frameCount - 1)
					finished.push_back(entity);
			});

		for (const Entity entity : finished)
			registry.DestroyEntity(entity);
	}
}