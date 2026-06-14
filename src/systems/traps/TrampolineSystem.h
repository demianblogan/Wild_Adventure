#pragma once

#include "audio/Mixer.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	class TrampolineSystem
	{
	public:
		TrampolineSystem(Registry& registry, Audio::Mixer& mixer);
		void Update();

	private:
		Registry&     registry;
		Audio::Mixer& mixer;
	};
}
