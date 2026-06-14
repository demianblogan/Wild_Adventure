#include "TrampolineSystem.h"

#include "components/Animation.h"
#include "components/AnimationState.h"
#include "components/Trampoline.h"

namespace ECS
{
	TrampolineSystem::TrampolineSystem(Registry& registry, Audio::Mixer& mixer)
		: registry(registry)
		, mixer(mixer)
	{}

	void TrampolineSystem::Update()
	{
		registry.ForEach<Trampoline, Animation, AnimationState>(
			[this](Entity, Trampoline& trampoline, Animation& animation, AnimationState& state)
			{
				if (trampoline.wasBounced)
				{
					trampoline.wasBounced = false;
					state.current = "Jump";

					// Restart from the first frame if the previous bounce is still playing.
					if (animation.playingState == "Jump")
					{
						animation.currentFrame = 0;
						animation.elapsedTime = 0.0f;
						animation.isFinished = false;
					}

					mixer.PlaySound("player_jump");
					return;
				}

				if (animation.playingState == "Jump" && animation.isFinished)
					state.current = "Idle";
			});
	}
}
