#pragma once

#include "audio/Mixer.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	class EnemySystem
	{
	public:
		EnemySystem(Registry& registry, int& score, Audio::Mixer& mixer);
		void Update();

	private:
		Registry&     registry;
		int&          score;
		Audio::Mixer& mixer;
	};
}
