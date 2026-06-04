#include "JumpSystem.h"

#include "components/CollisionState.h"
#include "components/Jump.h"
#include "components/Player.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	namespace
	{
		constexpr float WALL_JUMP_CONTROL_LOCK = 0.12f; // seconds the diagonal push is protected from input
	}

	JumpSystem::JumpSystem(Registry& registry)
		: registry(registry)
	{}

	void JumpSystem::Update()
	{
		registry.ForEach<Player, Velocity, Jump, CollisionState>(
			[](Entity, Player&, Velocity& velocity, Jump& jump, CollisionState& collisionState)
			{
				// Jumps refill only on the ground.
				if (collisionState.isOnGround)
					jump.jumpsRemaining = jump.maxJumps;

				if (jump.wantsToJump)
				{
					if (collisionState.isOnWall && !collisionState.isOnGround)
					{
						// Wall jump: always available while clinging, pushes up and away,
						// and spends a jump so it can't grant a free extra mid-air jump.
						velocity.y = -jump.jumpSpeed;
						velocity.x = jump.wallJumpPushX * -collisionState.wallDirection;
						jump.lockTimer = WALL_JUMP_CONTROL_LOCK;
						jump.jumpsRemaining = jump.maxJumps - 1;
					}
					else if (jump.jumpsRemaining > 0)
					{
						velocity.y = -jump.jumpSpeed;
						jump.jumpsRemaining--;
					}
				}

				jump.wantsToJump = false;
			});
	}
}