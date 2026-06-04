#include "JumpSystem.h"

#include "components/CollisionState.h"
#include "components/Jump.h"
#include "components/Player.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	JumpSystem::JumpSystem(Registry& registry)
		: registry(registry)
	{}

	void JumpSystem::Update()
	{
		registry.ForEach<Player, Velocity, Jump, CollisionState>(
			[](Entity, Player&, Velocity& velocity, Jump& jump, CollisionState& collisionState)
			{
				if (collisionState.isOnGround)
					jump.jumpsRemaining = jump.maxJumps;

				if (jump.wantsToJump && jump.jumpsRemaining > 0)
				{
					velocity.y = -jump.jumpSpeed;
					jump.jumpsRemaining--;
					collisionState.isOnGround = false;
				}

				jump.wantsToJump = false;
			});
	}
}