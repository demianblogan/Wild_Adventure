#pragma once

#include "core/ecs/Entity.h"

#include <string>

class DataLoader;
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
		BoxSystem(Registry& registry, DataLoader& loader, ParticleSystem& particles, Audio::Mixer& mixer);

		void Update();

	private:
		void EjectFruit(const std::string& fruitName, float x, float y, float ejectSpeedX, float ejectSpeedUp);

		Registry& registry;
		DataLoader& loader;
		ParticleSystem& particles;
		Audio::Mixer& mixer;
	};
}