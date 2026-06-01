#pragma once

namespace ECS
{
	class Registry;

	class AnimationSystem
	{
	public:
		AnimationSystem(Registry& registry);

		void Update(float deltaTime);

	private:
		Registry& registry;
	};
}