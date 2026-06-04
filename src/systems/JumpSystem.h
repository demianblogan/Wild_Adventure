#pragma once

namespace ECS
{
	class Registry;

	class JumpSystem
	{
	public:
		JumpSystem(Registry& registry);

		void Update();

	private:
		Registry& registry;
	};
}