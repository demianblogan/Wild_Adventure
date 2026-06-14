#pragma once

namespace ECS
{
	class Registry;

	class PatrolSystem
	{
	public:
		PatrolSystem(Registry& registry);

		void Update();

	private:
		Registry& registry;
	};
}