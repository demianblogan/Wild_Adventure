#pragma once

#include "components/Collider.h"
#include "components/Transform.h"
#include "core/ecs/Registry.h"
#include "tilemap/Tilemap.h"

class ParticleSystem;

namespace ECS
{
	class GroundPatrolSystem
	{
	public:
		GroundPatrolSystem(Registry& registry, const Tilemap& tilemap, ParticleSystem& particles);
		void Update(float deltaTime);

	private:
		bool HasGroundAhead(const Transform& transform, const Collider& collider, int direction) const;

		Registry&       registry;
		const Tilemap&  tilemap;
		ParticleSystem& particles;
	};
}
