#pragma once

namespace sf
{
	class RenderWindow;
}

class StateMachine;
class Resources;

struct Context
{
	sf::RenderWindow& window;
	StateMachine& stateMachine;
	Resources& resources;
};