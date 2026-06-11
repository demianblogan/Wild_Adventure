#include "Application.h"

#include "states/CompanySplashState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>
#include <cmath>
#include <format>
#include <memory>
#include <string>

Application::Application()
	: desktopMode(sf::VideoMode::getDesktopMode())
	, audioMixer(resources)
	, context(virtualScreen, stateMachine, resources, audioMixer, input, settings, *this)
{
	settings.Load("data/settings.json");

	CreateWindow();

	audioMixer.LoadFromFile("data/audio.json");
	audioMixer.SetSoundVolume(settings.GetSoundVolume() / 10.0f);
	audioMixer.SetMusicVolume(settings.GetMusicVolume() / 10.0f);

	input.LoadConfig("data/input.json");
	input.LoadDefaults("data/input_default.json");

	resources.textures.Load("cursor", "assets/textures/cursor/pointer.png");
	resources.textures.Get("cursor").setSmooth(false);

	RegisterInitialState();
}

void Application::CreateWindow()
{
	const ScreenMode mode = settings.GetScreenMode();

	if (mode == ScreenMode::Fullscreen)
	{
		const sf::VideoMode videoMode({ static_cast<unsigned int>(settings.GetResolutionWidth()),
			static_cast<unsigned int>(settings.GetResolutionHeight()) });
		window.create(videoMode, "2D Platformer", sf::Style::None, sf::State::Fullscreen);
	}
	else if (mode == ScreenMode::Window)
	{
		const sf::VideoMode videoMode({ static_cast<unsigned int>(settings.GetResolutionWidth()),
			static_cast<unsigned int>(settings.GetResolutionHeight()) });
		window.create(videoMode, "2D Platformer", sf::Style::Default, sf::State::Windowed);
	}
	else // Borderless: desktop-sized, no frame
	{
		window.create(desktopMode, "2D Platformer", sf::Style::None, sf::State::Windowed);
	}

	window.setVerticalSyncEnabled(settings.GetVsync());
	window.setMouseCursorVisible(false);

	appliedWidth = settings.GetResolutionWidth();
	appliedHeight = settings.GetResolutionHeight();
	appliedMode = settings.GetScreenMode();
}

void Application::ApplyGraphics()
{
	const bool changed = appliedWidth != settings.GetResolutionWidth()
		|| appliedHeight != settings.GetResolutionHeight()
		|| appliedMode != settings.GetScreenMode();

	if (changed)
		CreateWindow();   // resolution/mode changed: recreate (also refreshes vsync)
	else
		ApplyVsync();     // nothing visual changed: just keep vsync in sync
}

void Application::ApplyVsync()
{
	window.setVerticalSyncEnabled(settings.GetVsync());
}

void Application::SetCursorVisible(bool visible)
{
	cursorVisible = visible;
}

void Application::RegisterInitialState()
{
	stateMachine.Push(std::make_unique<CompanySplashState>(context));
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

		sf::Clock phaseClock;

		ProcessEvents();
		accumulatedEventsTime += phaseClock.restart().asSeconds();

		while (remainderTime >= FIXED_DELTA_TIME)
		{
			Update(FIXED_DELTA_TIME);
			remainderTime -= FIXED_DELTA_TIME;
		}
		accumulatedUpdateTime += phaseClock.restart().asSeconds();

		if (!stateMachine.IsEmpty())
			hasStateRun = true;
		else if (hasStateRun)
		{
			window.close();
			break;
		}

		UpdateFps(frameTime);

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

		stateMachine.HandleEvent(*event);
	}
}

void Application::Update(float deltaTime)
{
	input.Update();
	stateMachine.Update(deltaTime);
}

void Application::UpdateFps(float deltaTime)
{
	fpsTimer += deltaTime;
	fpsFrameCount++;

	if (fpsTimer >= 0.5f)
	{
		fpsDisplayed = static_cast<int>(std::round(fpsFrameCount / fpsTimer));

		const float toAverageMs = 1000.0f / static_cast<float>(fpsFrameCount);
		shownEventsMs = accumulatedEventsTime * toAverageMs;
		shownUpdateMs = accumulatedUpdateTime * toAverageMs;
		shownDrawMs = accumulatedDrawTime * toAverageMs;
		shownDisplayMs = accumulatedDisplayTime * toAverageMs;

		accumulatedEventsTime = 0.0f;
		accumulatedUpdateTime = 0.0f;
		accumulatedDrawTime = 0.0f;
		accumulatedDisplayTime = 0.0f;

		fpsTimer = 0.0f;
		fpsFrameCount = 0;
	}
}

void Application::Render(float interpolationFactor)
{
	sf::Clock phaseClock;

	virtualScreen.Clear();
	stateMachine.Render(interpolationFactor);
	DrawFps();
	virtualScreen.Display();

	window.clear(sf::Color::Black);
	virtualScreen.RenderToWindow(window);
	DrawCursor();
	accumulatedDrawTime += phaseClock.restart().asSeconds();

	window.display();
	accumulatedDisplayTime += phaseClock.getElapsedTime().asSeconds();
}

void Application::DrawFps()
{
	if (!settings.GetShowFps())
		return;

	if (!resources.fonts.Has("main"))
		return;

	virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);

	// Per-phase costs in ms: evt = window events, upd = game logic, draw = all
	// drawing, sync = waiting inside window.display() (vsync/driver pacing).
	const std::string overlay = std::format("FPS: {}  evt {:.1f} | upd {:.1f} | draw {:.1f} | sync {:.1f}",
		fpsDisplayed, shownEventsMs, shownUpdateMs, shownDrawMs, shownDisplayMs);

	sf::Text text(resources.fonts.Get("main"), overlay, 16);
	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color(40, 40, 40));
	text.setOutlineThickness(1.0f);

	const sf::FloatRect bounds = text.getLocalBounds();
	text.setPosition({ std::floor(VirtualScreen::WIDTH - bounds.size.x - 6.0f), std::floor(VirtualScreen::HEIGHT - 20.0f) });

	virtualScreen.GetRenderTarget().draw(text);
}

void Application::DrawCursor()
{
	if (!cursorVisible || input.GetActiveDevice() != InputDevice::Mouse)
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