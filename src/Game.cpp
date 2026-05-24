#include "Game.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

Game::Game()
	: desktopMode(sf::VideoMode::getDesktopMode())
{
	CreateWindow();
}

void Game::CreateWindow()
{
	window.create(desktopMode, "2D Platformer", sf::Style::None, sf::State::Windowed);
	window.setFramerateLimit(60);
}

void Game::Run()
{
	sf::Clock clock;
	float remainderTime = 0.0f;

	while (window.isOpen())
	{
		float frameTime = clock.restart().asSeconds();
		if (frameTime > MAX_FRAME_TIME)
			frameTime = MAX_FRAME_TIME;

		remainderTime += frameTime;

		ProcessEvents();

		while (remainderTime >= FIXED_DELTA_TIME)
		{
			Update(FIXED_DELTA_TIME);
			remainderTime -= FIXED_DELTA_TIME;
		}

		const float interpolationFactor = remainderTime / FIXED_DELTA_TIME;
		Render(interpolationFactor);
	}
}

void Game::ProcessEvents()
{
	while (const std::optional event = window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
			window.close();

		if (const auto* key = event->getIf<sf::Event::KeyPressed>())
		{
			if (key->code == sf::Keyboard::Key::Escape)
				window.close();
		}
	}
}

void Game::Update(float deltaTime)
{}

void Game::Render(float interpolationFactor)
{
	window.clear();
	window.display();
}