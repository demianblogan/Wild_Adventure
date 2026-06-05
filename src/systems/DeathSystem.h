#pragma once

#include <random>

namespace ECS
{
	class Registry;

	class DeathSystem
	{
	public:
		DeathSystem(Registry& registry);

		void Update(float deltaTime);

	private:
		Registry& registry;
		std::mt19937 randomEngine;
	};
}