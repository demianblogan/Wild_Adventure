#pragma once

class Input;

namespace ECS
{
	class Registry;

	class InputSystem
	{
	public:
		InputSystem(Registry& registry, const Input& input);

		void Update(float deltaTime);

	private:
		Registry& registry;
		const Input& input;
	};
}