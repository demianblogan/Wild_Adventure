#include "Application.h"

#include "states/TestState.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <memory>

Application::Application()
	: desktopMode(sf::VideoMode::getDesktopMode())
	, context(window, stateMachine, resources)
{
	CreateWindow();
	RegisterInitialState();
}

void Application::CreateWindow()
{
	window.create(desktopMode, "2D Platformer", sf::Style::None, sf::State::Windowed);
	window.setFramerateLimit(60);
}

void Application::RegisterInitialState()
{
	stateMachine.Push(std::make_unique<TestState>(context));
}

void Application::Run()
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

void Application::ProcessEvents()
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

		stateMachine.HandleEvent(*event);
	}
}

void Application::Update(float deltaTime)
{
	stateMachine.Update(deltaTime);
}

void Application::Render(float interpolationFactor)
{
	window.clear();
	stateMachine.Render(interpolationFactor);
	window.display();
}