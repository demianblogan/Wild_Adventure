#pragma once

#include "Context.h"
#include "core/Resources.h"
#include "core/StateMachine.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/VideoMode.hpp>

class Application
{
public:
	Application();

	void Run();

private:
	void CreateWindow();
	void RegisterInitialState();
	void ProcessEvents();
	void Update(float deltaTime);
	void Render(float interpolationFactor);

	static constexpr float FIXED_DELTA_TIME = 1.0f / 60.0f;
	static constexpr float MAX_FRAME_TIME = 0.25f;

	sf::VideoMode desktopMode;
	sf::RenderWindow window;
	StateMachine stateMachine;
	Resources resources;
	Context context;
};