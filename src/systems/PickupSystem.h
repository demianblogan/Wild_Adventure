#pragma once

namespace ECS
{
	class Registry;

	class PickupSystem
	{
	public:
		PickupSystem(Registry& registry, int& score);

		void Update(float deltaTime);

	private:
		Registry& registry;
		int& score;
	};
}