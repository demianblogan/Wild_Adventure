#pragma once

namespace ECS
{
	class Registry;

	class DamageSystem
	{
	public:
		DamageSystem(Registry& registry);

		void Update(float deltaTime);

	private:
		Registry& registry;
	};
}