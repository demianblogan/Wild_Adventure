#pragma once

class ParticleSystem;

namespace ECS
{
	class Registry;

	class AnimationSystem
	{
	public:
		AnimationSystem(Registry& registry, ParticleSystem* particles = nullptr);

		void Update(float deltaTime);

	private:
		Registry& registry;
		ParticleSystem* particles;
	};
}
