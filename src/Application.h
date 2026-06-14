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
	void DrawFps();
	void UpdateFps(float deltaTime);

	static constexpr float FIXED_DELTA_TIME = 1.0f / 60.0f;
	static constexpr float MAX_FRAME_TIME = 0.25f;

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

	float fpsTimer = 0.0f;
	int fpsFrameCount = 0;
	int fpsDisplayed = 0;

	int appliedWidth = 0;
	int appliedHeight = 0;
	ScreenMode appliedMode = ScreenMode::Borderless;

	bool cursorVisible = true;
};