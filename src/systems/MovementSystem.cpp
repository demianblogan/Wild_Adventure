#include "MovementSystem.h"

#include "components/PreviousTransform.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

MovementSystem::MovementSystem(Registry& registry)
	: registry(registry)
{}

void MovementSystem::Update(float deltaTime)
{
	registry.ForEach<Transform, PreviousTransform, Velocity>(
		[deltaTime](Entity entity, Transform& transform, PreviousTransform& previous, Velocity& velocity)
		{
			previous.x = transform.x;
			previous.y = transform.y;

			transform.x += velocity.x * deltaTime;
			transform.y += velocity.y * deltaTime;
		});
}