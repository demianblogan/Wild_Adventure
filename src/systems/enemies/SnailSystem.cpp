#include "SnailSystem.h"

#include "components/Animation.h"
#include "components/EnemyDeath.h"
#include "components/Facing.h"
#include "components/PreviousTransform.h"
#include "components/Shell.h"
#include "components/SnailAI.h"
#include "components/Stomped.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/DataLoader.h"
#include "core/ecs/Registry.h"

#include <vector>

namespace ECS
{
	namespace
	{
		constexpr float SHELL_KICK_GRACE = 0.3f; // keep the shell un-kickable just after it appears
	}

	SnailSystem::SnailSystem(Registry& registry, DataLoader& loader)
		: registry(registry)
		, loader(loader)
	{}

	void SnailSystem::Update(float)
	{
		struct Split { float x, y; bool faceRight; };
		std::vector<Split>  splits;
		std::vector<Entity> toDestroy;

		registry.ForEach<SnailAI, Stomped, Transform, Velocity>(
			[&](Entity entity, SnailAI&, Stomped&, Transform& transform, Velocity& velocity)
			{
				// Hold the snail in place (gravity still keeps it grounded) while Hit plays.
				velocity.x = 0.0f;

				if (!registry.Has<Animation>(entity))
					return;

				const Animation& anim = registry.Get<Animation>(entity);
				if (anim.playingState == "Hit" && anim.isFinished)
				{
					const bool faceRight = registry.Has<Facing>(entity)
						&& registry.Get<Facing>(entity).isLookingRight;

					splits.push_back({transform.x, transform.y, faceRight});
					toDestroy.push_back(entity);
				}
			});

		// Spawn after iterating: SpawnFromPrefab creates entities and can reallocate pools.
		for (const Split& split : splits)
		{
			SpawnShell(split.x, split.y, split.faceRight);
			SpawnBody(split.x, split.y);
		}

		for (const Entity entity : toDestroy)
			registry.DestroyEntity(entity);
	}

	void SnailSystem::SpawnShell(float x, float y, bool faceRight)
	{
		const Entity shell = loader.SpawnFromPrefab(registry, "data/prefabs/snail_shell.json");

		registry.Get<Transform>(shell) = { x, y };
		if (registry.Has<PreviousTransform>(shell))
			registry.Get<PreviousTransform>(shell) = { x, y };
		if (registry.Has<Facing>(shell))
			registry.Get<Facing>(shell).isLookingRight = faceRight;
		if (registry.Has<Shell>(shell))
			registry.Get<Shell>(shell).kickGrace = SHELL_KICK_GRACE;
	}

	void SnailSystem::SpawnBody(float x, float y)
	{
		const Entity body = loader.SpawnFromPrefab(registry, "data/prefabs/snail_body.json");

		registry.Get<Transform>(body) = { x, y };
		if (registry.Has<PreviousTransform>(body))
			registry.Get<PreviousTransform>(body) = { x, y };

		// Same exit as any stomped enemy: a brief pause, then bounce, spin, and fall away.
		registry.Add<EnemyDeath>(body, {});
	}
}
