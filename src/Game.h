#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/VideoMode.hpp>

class Game
{
public:
	Game();

	void Run();

private:
	void CreateWindow();
	void ProcessEvents();
	void Update(float deltaTime);
	void Render(float interpolationFactor);

	static constexpr float FIXED_DELTA_TIME = 1.0f / 60.0f;
	static constexpr float MAX_FRAME_TIME = 0.25f;

	sf::VideoMode desktopMode;
	sf::RenderWindow window;
};