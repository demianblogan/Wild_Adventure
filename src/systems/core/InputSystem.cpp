#include "InputSystem.h"

#include "components/combat/Health.h"
#include "components/physics/Jump.h"
#include "components/tags/Player.h"
#include "components/physics/Velocity.h"
#include "components/tags/Frozen.h"
#include "core/Input.h"
#include "core/ecs/Registry.h"

#include <algorithm>

namespace ECS
{
	namespace
	{
		float Approach(float current, float target, float maxDelta)
		{
			if (current < target)
				return std::min(current + maxDelta, target);
			if (current > target)
				return std::max(current - maxDelta, target);
			return target;
		}
	}

	InputSystem::InputSystem(Registry& registry, const Input& input)
		: registry(registry)
		, input(input)
	{}

	void InputSystem::Update(float deltaTime)
	{
		const float direction = input.GetAxisX();
		const bool jumpPressed = input.WasPressed(Action::Jump);

		registry.ForEach<Player, Velocity, Jump, Health>(
			[this, direction, jumpPressed, deltaTime](Entity entity, Player& player, Velocity& velocity, Jump& jump, Health& health)
			{
				if (registry.Has<Frozen>(entity))
					return;

				if (health.current <= 0)
					return;

				if (jump.lockTimer > 0.0f)
					jump.lockTimer -= deltaTime;

				// Control is locked during a wall-jump push or during hit-stun.
				const bool locked = (jump.lockTimer > 0.0f) || (health.hitStunTimer > 0.0f);

				if (!locked)
				{
					const float target = direction * player.moveSpeed;
					const float rate = (direction != 0.0f) ? player.acceleration : player.deceleration;
					velocity.x = Approach(velocity.x, target, rate * deltaTime);
				}

				if (jumpPressed)
					jump.wantsToJump = true;
			});
	}
}