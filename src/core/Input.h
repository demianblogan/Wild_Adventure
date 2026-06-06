#pragma once

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <string>
#include <vector>

enum class Action
{
	MoveLeft,
	MoveRight,
	Jump,
	MenuUp,
	MenuDown,
	MenuConfirm,
	MenuBack,
	Count
};

enum class InputDevice
{
	Mouse,
	Keyboard,
	Gamepad
};

class Input
{
public:
	void LoadConfig(const std::string& path);

	// Called once per fixed update step, before states update.
	void Update();

	bool IsDown(Action action) const;
	bool WasPressed(Action action) const;
	bool WasReleased(Action action) const;

	float GetAxisX() const; // -1, 0 or +1 (left/right movement)

	InputDevice GetActiveDevice() const { return activeDevice; }

	void NotifyMouseUsed() { activeDevice = InputDevice::Mouse; }

private:
	enum class BindingType
	{
		Key,
		Button,
		Axis
	};

	struct Binding
	{
		BindingType type = BindingType::Key;
		sf::Keyboard::Key key = sf::Keyboard::Key::Unknown;
		unsigned int button = 0;
		sf::Joystick::Axis axis = sf::Joystick::Axis::X;
		float direction = 0.0f; // sign for an axis binding
	};

	static int FindGamepad();
	bool IsBindingDown(const Binding& binding, int gamepad, bool& fromGamepad) const;

	static constexpr int ACTION_COUNT = static_cast<int>(Action::Count);

	std::vector<Binding> bindings[ACTION_COUNT];
	bool currentDown[ACTION_COUNT] = {};
	bool previousDown[ACTION_COUNT] = {};

	float axisThreshold = 50.0f; // percent of full axis travel
	InputDevice activeDevice = InputDevice::Keyboard;
};