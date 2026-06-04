#include "InputSystem.h"

#include "components/Jump.h"
#include "components/Player.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <SFML/Window/Keyboard.hpp>

namespace ECS
{
	InputSystem::InputSystem(Registry& registry)
		: registry(registry)
	{}

	void InputSystem::Update()
	{
		using Key = sf::Keyboard::Key;

		const bool left = sf::Keyboard::isKeyPressed(Key::A) || sf::Keyboard::isKeyPressed(Key::Left);
		const bool right = sf::Keyboard::isKeyPressed(Key::D) || sf::Keyboard::isKeyPressed(Key::Right);
		const float direction = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);

		const bool jumpDown = sf::Keyboard::isKeyPressed(Key::Space)
			|| sf::Keyboard::isKeyPressed(Key::W)
			|| sf::Keyboard::isKeyPressed(Key::Up);

		const bool jumpEdge = jumpDown && !wasJumpDown; // only on the press, not while held
		wasJumpDown = jumpDown;

		registry.ForEach<Player, Velocity, Jump>(
			[direction, jumpEdge](Entity, Player& player, Velocity& velocity, Jump& jump)
			{
				velocity.x = direction * player.moveSpeed;

				if (jumpEdge)
					jump.wantsToJump = true;
			});
	}
}