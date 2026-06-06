#pragma once

#include "core/ecs/Entity.h"

#include <random>
#include <string>

class DataLoader;
class ParticleSystem;

namespace ECS
{
	class Registry;

	class BoxSystem
	{
	public:
		BoxSystem(Registry& registry, DataLoader& loader, ParticleSystem& particles);

		void Update();

	private:
		void EjectFruit(const std::string& fruitName, float x, float y, float ejectSpeedX, float ejectSpeedUp);

		Registry& registry;
		DataLoader& loader;
		ParticleSystem& particles;
		std::mt19937 randomEngine;
	};
}