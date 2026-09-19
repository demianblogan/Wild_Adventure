#pragma once

#include "Context.h"
#include "core/Campaign.h"
#include "core/GraphicsTarget.h"
#include "core/Input.h"
#include "core/Settings.h"
#include "audio/Mixer.h"
#include "core/VirtualScreen.h"
#include "core/Resources.h"
#include "core/StateMachine.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/VideoMode.hpp>

class Application : public GraphicsTarget
{
public:
	Application();

	void Run();

	void ApplyGraphics() override;
	void ApplyVsync() override;
	void SetCursorVisible(bool visible) override;

private:
	void CreateWindow();
	void RegisterInitialState();
	void ProcessEvents();
	void Update(float deltaTime);
	void Render(float interpolationFactor);
	void DrawCursor();

	void UpdateFpsCounter(float frameTime);
	void DrawFpsCounter();

	static constexpr float FixedDeltaTime = 1.0f / 60.0f;
	static constexpr float MaxFrameTime = 0.25f;
	static constexpr float UnfocusedSleepInterval = 0.1f; // seconds to idle per loop while unfocused

	static constexpr float FpsUpdateInterval = 0.5f; // seconds between FPS label refreshes
	static constexpr unsigned int FpsTextSize = 40;
	static constexpr float FpsTextMargin = 12.0f;
	static constexpr float FpsTextOutlineThickness = 4.0f;

	sf::VideoMode desktopMode;
	sf::RenderWindow window;
	VirtualScreen virtualScreen;
	StateMachine stateMachine;
	Resources resources;
	Audio::Mixer audioMixer;
	Input input;
	Settings settings;
	Campaign campaign;
	Context context;

	int appliedWidth = 0;
	int appliedHeight = 0;
	ScreenMode appliedMode = ScreenMode::Borderless;

	bool isCursorVisible = true;
	bool isWindowFocused = true;

	float fpsUpdateTimer = 0.0f;
	int fpsFrameCount = 0;
	int displayedFps = 0;
};