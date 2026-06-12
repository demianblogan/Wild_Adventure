#pragma once

class ParticleSystem;

namespace ECS
{
	class Registry;

	class ChickenSystem
	{
	public:
		ChickenSystem(Registry& registry, ParticleSystem& particles);

		void Update(float deltaTime);

	private:
		Registry& registry;
		ParticleSystem& particles;
	};
}
