#include "TurtleSystem.h"

#include "components/Animation.h"
#include "components/AnimationState.h"
#include "components/EnemyDeath.h"
#include "components/Spiky.h"
#include "components/TurtleAI.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	TurtleSystem::TurtleSystem(Registry& registry)
		: registry(registry)
	{}

	void TurtleSystem::Update(float deltaTime)
	{
		registry.ForEach<TurtleAI, AnimationState>(
			[&](Entity entity, TurtleAI& turtle, AnimationState& animState)
			{
				// A stomped turtle dies like any other enemy; leave it to EnemyDeathSystem.
				if (registry.Has<EnemyDeath>(entity))
					return;

				switch (turtle.phase)
				{
				case TurtleAI::Phase::Safe:
					turtle.phaseTimer -= deltaTime;
					if (turtle.phaseTimer <= 0.0f)
					{
						// Spikes start coming out: armored from this moment on.
						registry.Add<Spiky>(entity, {});
						turtle.phase      = TurtleAI::Phase::SpikesEmerging;
						animState.current = "SpikesOut";
					}
					break;

				case TurtleAI::Phase::SpikesEmerging:
					if (registry.Has<Animation>(entity))
					{
						const Animation& anim = registry.Get<Animation>(entity);
						if (anim.playingState == "SpikesOut" && anim.isFinished)
						{
							turtle.phase      = TurtleAI::Phase::Spiked;
							turtle.phaseTimer = TurtleAI::SPIKED_DURATION;
							animState.current = "Idle1";
						}
					}
					break;

				case TurtleAI::Phase::Spiked:
					turtle.phaseTimer -= deltaTime;
					if (turtle.phaseTimer <= 0.0f)
					{
						turtle.phase      = TurtleAI::Phase::SpikesRetracting;
						animState.current = "SpikesIn";
					}
					break;

				case TurtleAI::Phase::SpikesRetracting:
					if (registry.Has<Animation>(entity))
					{
						const Animation& anim = registry.Get<Animation>(entity);
						if (anim.playingState == "SpikesIn" && anim.isFinished)
						{
							// Spikes fully retracted: vulnerable to a stomp again.
							registry.RemoveFrom<Spiky>(entity);
							turtle.phase      = TurtleAI::Phase::Safe;
							turtle.phaseTimer = TurtleAI::SAFE_DURATION;
							animState.current = "Idle2";
						}
					}
					break;
				}
			});
	}
}
