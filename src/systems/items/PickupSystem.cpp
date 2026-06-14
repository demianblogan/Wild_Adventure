#include "PickupSystem.h"

#include "components/render/Animation.h"
#include "components/render/AnimationState.h"
#include "components/items/Collectible.h"
#include "components/physics/Collider.h"
#include "components/tags/Despawning.h"
#include "components/physics/Hitbox.h"
#include "components/items/PickupDelay.h"
#include "components/physics/Transform.h"
#include "core/AABB.h"
#include "core/ecs/Registry.h"
#include "systems/core/PlayerQuery.h"

#include <vector>

namespace ECS
{
	PickupSystem::PickupSystem(Registry& registry, int& score, int& fruitsCollected)
		: registry(registry)
		, score(score)
		, fruitsCollected(fruitsCollected)
	{}

	void PickupSystem::Update(float deltaTime)
	{
		const Entity playerEntity = FindPlayer(registry);
		if (playerEntity == INVALID_ENTITY)
			return;

		const Transform& playerTransform = registry.Get<Transform>(playerEntity);
		const Collider&  playerCollider  = registry.Get<Collider>(playerEntity);
		const AABB playerBox = FeetAABB(playerTransform.x, playerTransform.y,
			playerCollider.width, playerCollider.height);

		std::vector<Entity> collected;

		registry.ForEach<Collectible, Hitbox, Transform>(
			[&](Entity entity, Collectible&, Hitbox& hitbox, Transform& transform)
			{
				// Freshly ejected fruit can't be grabbed until its delay runs out.
				if (registry.Has<PickupDelay>(entity))
				{
					PickupDelay& delay = registry.Get<PickupDelay>(entity);
					if (delay.timer > 0.0f)
					{
						delay.timer -= deltaTime;
						return;
					}
				}

				const AABB fruitBox = FeetAABB(transform.x, transform.y, hitbox.width, hitbox.height);
				if (playerBox.Overlaps(fruitBox))
					collected.push_back(entity);
			});

		for (const Entity entity : collected)
		{
			score += registry.Get<Collectible>(entity).points;
			fruitsCollected++;

			registry.RemoveFrom<Collectible>(entity);
			if (registry.Has<Hitbox>(entity))
				registry.RemoveFrom<Hitbox>(entity);

			if (registry.Has<AnimationState>(entity))
				registry.Get<AnimationState>(entity).current = "Collected";

			registry.Add<Despawning>(entity, {});
		}

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