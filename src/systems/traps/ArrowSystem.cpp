#include "ArrowSystem.h"

#include "audio/Mixer.h"
#include "components/Animation.h"
#include "components/AnimationState.h"
#include "components/Arrow.h"
#include "components/Collider.h"
#include "components/Hitbox.h"
#include "components/Jump.h"
#include "components/Player.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <vector>

namespace ECS
{
	ArrowSystem::ArrowSystem(Registry& registry, Audio::Mixer& mixer)
		: registry(registry)
		, mixer(mixer)
	{}

	void ArrowSystem::Update()
	{
		Entity playerEntity = INVALID_ENTITY;
		registry.ForEach<Player>([&](Entity entity, Player&) { playerEntity = entity; });
		if (playerEntity == INVALID_ENTITY)
			return;

		const Transform& playerTransform = registry.Get<Transform>(playerEntity);
		const Collider&  playerCollider  = registry.Get<Collider>(playerEntity);
		Velocity&        playerVelocity  = registry.Get<Velocity>(playerEntity);

		const float pHalfW  = playerCollider.width / 2.0f;
		const float pLeft   = playerTransform.x - pHalfW;
		const float pRight  = playerTransform.x + pHalfW;
		const float pTop    = playerTransform.y - playerCollider.height;
		const float pBottom = playerTransform.y;

		// ArrowSystem never creates entities, so holding player references across the
		// loop is safe; arrows are removed after iterating.
		std::vector<Entity> toDestroy;

		registry.ForEach<Arrow, Transform, Hitbox, AnimationState>(
			[&](Entity entity, Arrow& arrow, Transform& transform, Hitbox& hitbox, AnimationState& animState)
			{
				if (!arrow.triggered)
				{
					const float hHalfW  = hitbox.width / 2.0f;
					const float aLeft   = transform.x - hHalfW;
					const float aRight  = transform.x + hHalfW;
					const float aTop    = transform.y - hitbox.height;
					const float aBottom = transform.y;

					const bool overlap = pLeft < aRight && pRight > aLeft
						&& pTop < aBottom && pBottom > aTop;

					if (overlap)
					{
						// Always launch straight up; keep the player's horizontal motion.
						playerVelocity.y = -arrow.boostSpeed;

						// Like the trampoline, grant an air jump so the player keeps control.
						if (registry.Has<Jump>(playerEntity)
							&& registry.Get<Jump>(playerEntity).jumpsRemaining < 1)
							registry.Get<Jump>(playerEntity).jumpsRemaining = 1;

						animState.current = "Hit";
						arrow.triggered   = true;
						mixer.PlaySound("player_jump");
					}
				}
				else if (registry.Has<Animation>(entity))
				{
					// Used: despawn for good once the Hit animation finishes.
					const Animation& anim = registry.Get<Animation>(entity);
					if (anim.playingState == "Hit" && anim.isFinished)
						toDestroy.push_back(entity);
				}
			});

		for (const Entity entity : toDestroy)
			registry.DestroyEntity(entity);
	}
}
