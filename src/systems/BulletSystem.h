#pragma once

#include "core/ecs/Registry.h"
#include "tilemap/Tilemap.h"

class ParticleSystem;

namespace ECS
{
	class BulletSystem
	{
	public:
		BulletSystem(Registry& registry, const Tilemap& tilemap, ParticleSystem& particles);
		void Update(float deltaTime);

	private:
		Registry&       registry;
		const Tilemap&  tilemap;
		ParticleSystem& particles;
	};
}
