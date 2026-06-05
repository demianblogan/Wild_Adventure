#include "InputSystem.h"

#include "components/Health.h"
#include "components/Jump.h"
#include "components/Player.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <SFML/Window/Keyboard.hpp>

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

	InputSystem::InputSystem(Registry& registry)
		: registry(registry)
	{}

	void InputSystem::Update(float deltaTime)
	{
		using Key = sf::Keyboard::Key;

		const bool left = sf::Keyboard::isKeyPressed(Key::A) || sf::Keyboard::isKeyPressed(Key::Left);
		const bool right = sf::Keyboard::isKeyPressed(Key::D) || sf::Keyboard::isKeyPressed(Key::Right);
		const float direction = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);

		const bool jumpDown = sf::Keyboard::isKeyPressed(Key::Space)
			|| sf::Keyboard::isKeyPressed(Key::W)
			|| sf::Keyboard::isKeyPressed(Key::Up);

		const bool jumpEdge = jumpDown && !wasJumpDown;
		wasJumpDown = jumpDown;

		registry.ForEach<Player, Velocity, Jump, Health>(
			[direction, jumpEdge, deltaTime](Entity, Player& player, Velocity& velocity, Jump& jump, Health& health)
			{
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

				if (jumpEdge)
					jump.wantsToJump = true;
			});
	}
}