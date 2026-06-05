#pragma once

namespace ECS
{
	class Registry;

	class PickupSystem
	{
	public:
		PickupSystem(Registry& registry, int& score);

		void Update();

	private:
		Registry& registry;
		int& score;
	};
}