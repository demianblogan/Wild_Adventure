#include "MovementSystem.h"

#include "components/Collider.h"
#include "components/Facing.h"
#include "components/PreviousTransform.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	MovementSystem::MovementSystem(Registry& registry)
		: registry(registry)
	{}

	void MovementSystem::Update(float deltaTime)
	{
		registry.ForEach<Transform, PreviousTransform, Velocity>(
			[this, deltaTime](Entity entity, Transform& transform, PreviousTransform& previous, Velocity& velocity)
			{
				// Entities with a collider are driven by the physics system instead.
				if (registry.Has<Collider>(entity))
					return;

				previous.x = transform.x;
				previous.y = transform.y;

				transform.x += velocity.x * deltaTime;
				transform.y += velocity.y * deltaTime;

				if (registry.Has<Facing>(entity) && velocity.x != 0.0f)
					registry.Get<Facing>(entity).isLookingRight = velocity.x > 0.0f;
			});
	}
}