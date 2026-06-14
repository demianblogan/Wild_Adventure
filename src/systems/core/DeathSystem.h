#pragma once

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
	};
}