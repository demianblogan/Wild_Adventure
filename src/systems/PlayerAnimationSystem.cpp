#include "PlayerAnimationSystem.h"

#include "components/AnimationState.h"
#include "components/CollisionState.h"
#include "components/Jump.h"
#include "components/Player.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <string>

namespace ECS
{
	PlayerAnimationSystem::PlayerAnimationSystem(Registry& registry)
		: registry(registry)
	{}

	void PlayerAnimationSystem::Update()
	{
		registry.ForEach<Player, Velocity, CollisionState, AnimationState, Jump>(
			[](Entity, Player&, Velocity& velocity, CollisionState& collisionState,
				AnimationState& state, Jump& jump)
			{
				std::string desired;

				if (!collisionState.isOnGround)
				{
					if (velocity.y < 0.0f)
						desired = (jump.jumpsRemaining == 0) ? "DoubleJump" : "Jump";
					else
						desired = "Fall";
				}
				else if (velocity.x != 0.0f)
				{
					desired = "Run";
				}
				else
				{
					desired = "Idle";
				}

				if (state.current != desired)
					state.current = desired;
			});
	}
}