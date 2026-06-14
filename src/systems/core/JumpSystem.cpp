#include "JumpSystem.h"

#include "components/physics/CollisionState.h"
#include "components/physics/Jump.h"
#include "components/tags/Player.h"
#include "components/physics/Velocity.h"
#include "core/ecs/Registry.h"

#include <algorithm>

namespace ECS
{
	namespace
	{
		constexpr float WALL_JUMP_CONTROL_LOCK = 0.12f; // seconds the diagonal push is protected from input

		// Chained wall jumps add to any upward speed the hero already has (capped at
		// this multiple of a normal jump) so climbing a wall with repeated jumps is
		// quick, instead of each absolute reset cancelling the previous jump's lift.
		constexpr float WALL_CLIMB_SPEED_CAP = 1.35f;
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
						// Add the lift to any rising momentum (falling counts as zero) and
						// cap it, so re-touching the wall mid-rise keeps climbing fast.
						velocity.y = std::max(std::min(velocity.y, 0.0f) - jump.jumpSpeed,
							-jump.jumpSpeed * WALL_CLIMB_SPEED_CAP);
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