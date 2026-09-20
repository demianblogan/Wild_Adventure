#include "TrampolineSystem.h"

#include "components/render/Animation.h"
#include "components/render/AnimationState.h"
#include "components/traps/Trampoline.h"
#include "core/GamepadHaptics.h"
#include "core/HapticCues.h"

namespace ECS
{
	TrampolineSystem::TrampolineSystem(Registry& registry, Audio::Mixer& mixer, Haptics::GamepadHaptics& gamepadHaptics)
		: registry(registry)
		, mixer(mixer)
		, gamepadHaptics(gamepadHaptics)
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
					Haptics::PulseLaunch(gamepadHaptics);
					return;
				}

				if (animation.playingState == "Jump" && animation.isFinished)
					state.current = "Idle";
			});
	}
}
