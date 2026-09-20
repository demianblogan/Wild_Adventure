#pragma once

#include "audio/Mixer.h"
#include "core/ecs/Registry.h"

namespace Haptics
{
	class GamepadHaptics;
}

namespace ECS
{
	class TrampolineSystem
	{
	public:
		TrampolineSystem(Registry& registry, Audio::Mixer& mixer, Haptics::GamepadHaptics& gamepadHaptics);
		void Update();

	private:
		Registry&     registry;
		Audio::Mixer& mixer;
		Haptics::GamepadHaptics& gamepadHaptics;
	};
}
