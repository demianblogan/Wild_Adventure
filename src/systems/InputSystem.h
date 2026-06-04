#pragma once

namespace ECS
{
	class Registry;

	class InputSystem
	{
	public:
		InputSystem(Registry& registry);

		void Update(float deltaTime);

	private:
		Registry& registry;
		bool wasJumpDown = false;
	};
}