#pragma once

namespace ECS
{
	class Registry;

	class InputSystem
	{
	public:
		InputSystem(Registry& registry);

		void Update();

	private:
		Registry& registry;
		bool wasJumpDown = false;
	};
}