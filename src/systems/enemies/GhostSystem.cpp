#include "GhostSystem.h"

#include "components/Animation.h"
#include "components/AnimationState.h"
#include "components/Collider.h"
#include "components/EnemyDeath.h"
#include "components/Facing.h"
#include "components/GhostAI.h"
#include "components/Hidden.h"
#include "components/Transform.h"
#include "core/ecs/Registry.h"
#include "graphics/ParticleSystem.h"

namespace ECS
{
	GhostSystem::GhostSystem(Registry& registry, ParticleSystem& particles)
		: registry(registry)
		, particles(particles)
	{}

	void GhostSystem::Update(float deltaTime)
	{
		registry.ForEach<GhostAI, Transform, Collider, AnimationState, Facing>(
			[&](Entity entity, GhostAI& ghost, Transform& transform, Collider& collider,
				AnimationState& animState, Facing& facing)
			{
				// A stomped ghost dies like any other enemy; leave it to EnemyDeathSystem.
				if (registry.Has<EnemyDeath>(entity))
					return;

				switch (ghost.phase)
				{
				case GhostAI::Phase::Visible:
				{
					// Trail wisps from the back (opposite the facing/travel direction).
					ghost.particleTimer -= deltaTime;
					if (ghost.particleTimer <= 0.0f)
					{
						const int   facingDir = facing.isLookingRight ? 1 : -1;
						const float backX     = transform.x - static_cast<float>(facingDir) * (collider.width * 0.5f);
						const float midY      = transform.y - collider.height * 0.5f;

						particles.EmitGhostTrail({ backX, midY }, facingDir);
						ghost.particleTimer = GhostAI::PARTICLE_INTERVAL;
					}

					ghost.phaseTimer -= deltaTime;
					if (ghost.phaseTimer <= 0.0f)
					{
						animState.current = "Disappear"; // Disappearing waits for the clip to finish
						ghost.phase       = GhostAI::Phase::Disappearing;
					}
					break;
				}

				case GhostAI::Phase::Disappearing:
					if (registry.Has<Animation>(entity))
					{
						const Animation& anim = registry.Get<Animation>(entity);
						if (anim.playingState == "Disappear" && anim.isFinished)
						{
							// Now fully gone: hide from rendering and from combat, keep moving.
							registry.Add<Hidden>(entity, {});
							ghost.phase      = GhostAI::Phase::Invisible;
							ghost.phaseTimer = GhostAI::INVISIBLE_DURATION;
						}
					}
					break;

				case GhostAI::Phase::Invisible:
					ghost.phaseTimer -= deltaTime;
					if (ghost.phaseTimer <= 0.0f)
					{
						registry.RemoveFrom<Hidden>(entity);
						ghost.phase       = GhostAI::Phase::Appearing;
						animState.current = "Appear";
					}
					break;

				case GhostAI::Phase::Appearing:
					if (registry.Has<Animation>(entity))
					{
						const Animation& anim = registry.Get<Animation>(entity);
						if (anim.playingState == "Appear" && anim.isFinished)
						{
							ghost.phase       = GhostAI::Phase::Visible;
							ghost.phaseTimer  = GhostAI::VISIBLE_DURATION;
							animState.current = "Idle";
						}
					}
					break;
				}
			});
	}
}
