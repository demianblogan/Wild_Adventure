#pragma once

namespace ECS
{
	class Registry;

	class PlayerAnimationSystem
	{
	public:
		PlayerAnimationSystem(Registry& registry);

		void Update();

	private:
		Registry& registry;
	};
}