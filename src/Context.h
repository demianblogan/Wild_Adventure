#pragma once

namespace sf
{
	class RenderWindow;
}

class StateMachine;

struct Context
{
	sf::RenderWindow& window;
	StateMachine& stateMachine;
};