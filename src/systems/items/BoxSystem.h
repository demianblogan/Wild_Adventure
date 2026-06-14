#pragma once

#include "core/ecs/Entity.h"

#include <string>

class SceneLoader;
class ParticleSystem;

namespace Audio
{
	class Mixer;
}

namespace ECS
{
	class Registry;

	class BoxSystem
	{
	public:
		BoxSystem(Registry& registry, SceneLoader& loader, ParticleSystem& particles, Audio::Mixer& mixer);

		void Update();

	private:
		void EjectFruit(const std::string& fruitName, float x, float y, float ejectSpeedX, float ejectSpeedUp);

		Registry& registry;
		SceneLoader& loader;
		ParticleSystem& particles;
		Audio::Mixer& mixer;
	};
}