#include "Application.h"

#include "states/SplashState.h"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>
#include <cmath>
#include <memory>

Application::Application()
	: desktopMode(sf::VideoMode::getDesktopMode())
	, audioMixer(resources)
	, context(virtualScreen, stateMachine, resources, audioMixer, input, settings)
{
	CreateWindow();
	audioMixer.LoadFromFile("data/audio.json");
	input.LoadConfig("data/input.json");

	settings.Load("data/settings.json");
	audioMixer.SetSoundVolume(settings.GetSoundVolume() / 10.0f);
	audioMixer.SetMusicVolume(settings.GetMusicVolume() / 10.0f);

	resources.textures.Load("cursor", "assets/textures/cursor/pointer.png");
	resources.textures.Get("cursor").setSmooth(false);

	RegisterInitialState();
}

void Application::CreateWindow()
{
	window.create(desktopMode, "2D Platformer", sf::Style::None, sf::State::Windowed);
	window.setVerticalSyncEnabled(true);
	window.setMouseCursorVisible(false); // we draw our own pixel cursor instead
}

void Application::RegisterInitialState()
{
	stateMachine.Push(std::make_unique<SplashState>(context));
}

void Application::Run()
{
	sf::Clock clock;
	float remainderTime = 0.0f;
	bool hasStateRun = false;

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

		if (!stateMachine.IsEmpty())
			hasStateRun = true;
		else if (hasStateRun)
		{
			window.close();
			break;
		}

		const float interpolationFactor = remainderTime / FIXED_DELTA_TIME;
		Render(interpolationFactor);
	}
}

void Application::ProcessEvents()
{
	while (const std::optional event = window.pollEvent())
	{
		if (const auto* moved = event->getIf<sf::Event::MouseMoved>())
		{
			virtualScreen.UpdateMousePosition(moved->position, window);
			input.NotifyMouseUsed();
		}
		else if (const auto* pressed = event->getIf<sf::Event::MouseButtonPressed>())
		{
			virtualScreen.UpdateMousePosition(pressed->position, window);
			input.NotifyMouseUsed();
		}
		else if (const auto* released = event->getIf<sf::Event::MouseButtonReleased>())
		{
			virtualScreen.UpdateMousePosition(released->position, window);
			input.NotifyMouseUsed();
		}

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
	input.Update();
	stateMachine.Update(deltaTime);
}

void Application::Render(float interpolationFactor)
{
	virtualScreen.Clear();
	stateMachine.Render(interpolationFactor);
	virtualScreen.Display();

	window.clear(sf::Color::Black);
	virtualScreen.RenderToWindow(window);
	DrawCursor();
	window.display();
}

void Application::DrawCursor()
{
	if (input.GetActiveDevice() != InputDevice::Mouse)
		return;

	const sf::Vector2u windowSize = window.getSize();
	const float scale = std::min(
		static_cast<float>(windowSize.x) / VirtualScreen::WIDTH,
		static_cast<float>(windowSize.y) / VirtualScreen::HEIGHT);

	const sf::Vector2i mouse = sf::Mouse::getPosition(window);

	sf::Sprite cursor(resources.textures.Get("cursor"));
	cursor.setScale({ scale * 0.5f, scale * 0.5f });
	cursor.setPosition({ std::floor(static_cast<float>(mouse.x)), std::floor(static_cast<float>(mouse.y)) });

	window.draw(cursor);
}