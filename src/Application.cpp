#include "Application.h"

#include "core/AppDataPath.h"
#include "states/CompanySplashState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

Application::Application()
	: desktopMode(sf::VideoMode::getDesktopMode())
	, audioMixer(resources)
	, context(virtualScreen, stateMachine, resources, audioMixer, input, settings, *this, campaign, localization, gamepadHaptics)
{
	// Per-player data (settings, campaign progress, key bindings) lives under
	// %LOCALAPPDATA%, never inside the install/repo directory; only the
	// read-only defaults shipped with the game come from data/.
	settings.Load(AppDataPath::Resolve("settings.json").string());
	campaign.Load(AppDataPath::Resolve("save.json").string());
	localization.SetLanguage(settings.GetLanguage());

	CreateWindow();

	audioMixer.LoadFromFile("data/audio.json");
	audioMixer.SetSoundVolume(settings.GetSoundVolume() / 10.0f);
	audioMixer.SetMusicVolume(settings.GetMusicVolume() / 10.0f);

	// Defaults load first: LoadConfig falls back to them if the saved
	// bindings file turns out to be missing or corrupt.
	input.LoadDefaults("data/input_default.json");
	input.LoadConfig(AppDataPath::Resolve("input.json").string());

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

	window.setVerticalSyncEnabled(settings.IsVsyncEnabled());
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
	window.setVerticalSyncEnabled(settings.IsVsyncEnabled());
}

void Application::SetCursorVisible(bool visible)
{
	isCursorVisible = visible;
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
		UpdateFpsCounter(frameTime);

		if (frameTime > MaxFrameTime)
			frameTime = MaxFrameTime;

		ProcessEvents();

		if (isWindowFocused)
		{
			remainderTime += frameTime;

			while (remainderTime >= FixedDeltaTime)
			{
				Update(FixedDeltaTime);
				remainderTime -= FixedDeltaTime;
			}
		}
		else
		{
			// Unfocused: freeze gameplay instead of accumulating a catch-up
			// burst of updates for whenever the window regains focus, and
			// avoid busy-spinning the loop while there is nothing to do.
			remainderTime = 0.0f;
			sf::sleep(sf::seconds(UnfocusedSleepInterval));
		}

		if (!stateMachine.IsEmpty())
			hasStateRun = true;
		else if (hasStateRun)
		{
			window.close();
			break;
		}

		const float interpolationFactor = remainderTime / FixedDeltaTime;
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

		if (event->is<sf::Event::FocusLost>())
			isWindowFocused = false;
		else if (event->is<sf::Event::FocusGained>())
			isWindowFocused = true;

		if (isWindowFocused)
			stateMachine.HandleEvent(*event);
	}
}

void Application::Update(float deltaTime)
{
	input.Update();
	gamepadHaptics.Update(deltaTime);
	stateMachine.Update(deltaTime);
}

void Application::Render(float interpolationFactor)
{
	virtualScreen.Clear();
	stateMachine.Render(interpolationFactor);
	virtualScreen.Display();

	window.clear(sf::Color::Black);
	virtualScreen.RenderToWindow(window);

	if (settings.IsShowFpsEnabled())
		DrawFpsCounter();

	DrawCursor();

	window.display();
}

void Application::UpdateFpsCounter(float frameTime)
{
	if (frameTime <= 0.0f)
		return;

	fpsFrameCount++;
	fpsUpdateTimer += frameTime;

	if (fpsUpdateTimer >= FpsUpdateInterval)
	{
		displayedFps = static_cast<int>(std::round(static_cast<float>(fpsFrameCount) / fpsUpdateTimer));
		fpsFrameCount = 0;
		fpsUpdateTimer = 0.0f;
	}
}

void Application::DrawFpsCounter()
{
	if (!resources.fonts.Has("main"))
	{
		// Shares the button font's file: it is the only one of the three UI
		// fonts with Cyrillic glyphs, so "main" (used for most body text) has
		// to be backed by it too for Russian/Ukrainian to render at all.
		resources.fonts.Load("main", "assets/fonts/born2bsporty-fs.regular.otf");
		resources.fonts.Get("main").setSmooth(false);
	}

	sf::Text text(resources.fonts.Get("main"), std::to_string(displayedFps) + " FPS", FpsTextSize);
	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(FpsTextOutlineThickness);

	// getLocalBounds() excludes the glyphs' own bearing/overshoot, so anchoring
	// by size alone gives an inconsistent visual margin: correct for that
	// offset to keep the top and right margins visually equal.
	const sf::FloatRect textBounds = text.getLocalBounds();
	const float windowWidth = static_cast<float>(window.getSize().x);
	text.setPosition(
	{
		windowWidth - FpsTextMargin - (textBounds.position.x + textBounds.size.x),
		FpsTextMargin - textBounds.position.y
	});

	window.draw(text);
}

void Application::DrawCursor()
{
	if (!isCursorVisible || input.GetActiveDevice() != InputDevice::Mouse)
		return;

	const sf::Vector2u windowSize = window.getSize();
	const float scale = std::min(
		static_cast<float>(windowSize.x) / VirtualScreen::Width,
		static_cast<float>(windowSize.y) / VirtualScreen::Height);

	const sf::Vector2i mouse = sf::Mouse::getPosition(window);

	sf::Sprite cursor(resources.textures.Get("cursor"));
	cursor.setScale({ scale * 0.5f, scale * 0.5f });
	cursor.setPosition({ std::floor(static_cast<float>(mouse.x)), std::floor(static_cast<float>(mouse.y)) });

	window.draw(cursor);
}