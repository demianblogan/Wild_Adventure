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
	Pause,
	MenuUp,
	MenuDown,
	MenuLeft,
	MenuRight,
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
	void LoadDefaults(const std::string& path);

	// Called once per fixed update step, before states update.
	void Update();

	bool IsDown(Action action) const;
	bool WasPressed(Action action) const;
	bool WasReleased(Action action) const;

	float GetAxisX() const; // -1, 0 or +1 (left/right movement)

	InputDevice GetActiveDevice() const { return activeDevice; }

	void NotifyMouseUsed() { activeDevice = InputDevice::Mouse; }

	// Working/saved model for the keyboard rebinding page (mirrors Settings).
	bool IsDirty() const;
	void Revert();
	// Returns false if the write failed (e.g. disk full, file locked); the
	// bindings then stay dirty so IsDirty() keeps reporting unsaved changes
	// instead of the caller believing the save went through.
	bool SaveConfig(const std::string& path);
	void ResetToDefaults();

	// Keyboard rebinding works on a single primary key per action; gamepad
	// button/axis bindings are left untouched.
	sf::Keyboard::Key GetPrimaryKey(Action action) const;
	void SetPrimaryKey(Action action, sf::Keyboard::Key key);

	static std::string KeyName(sf::Keyboard::Key key);

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

		bool operator==(const Binding& other) const = default;
	};

	static constexpr int ActionCount = static_cast<int>(Action::Count);

	using BindingSet = std::vector<Binding>[ActionCount];

	static void LoadBindingsFile(const std::string& path, BindingSet target, float* outAxisThreshold);

	static int FindGamepad();
	bool IsBindingDown(const Binding& binding, int gamepad, bool isGamepadPlayStation, bool& fromGamepad) const;

	std::vector<Binding> bindings[ActionCount];        // working set used by Update
	std::vector<Binding> savedBindings[ActionCount];   // last persisted state
	std::vector<Binding> defaultBindings[ActionCount]; // factory defaults

	bool currentDown[ActionCount] = {};
	bool previousDown[ActionCount] = {};

	static constexpr float DefaultAxisThreshold = 50.0f; // percent of full axis travel

	float axisThreshold = DefaultAxisThreshold;
	InputDevice activeDevice = InputDevice::Keyboard;
};