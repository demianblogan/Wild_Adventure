#include "Input.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>
#include <unordered_map>

namespace
{
	const std::unordered_map<std::string, Action> ACTION_NAMES =
	{
		{ "MoveLeft", Action::MoveLeft },
		{ "MoveRight", Action::MoveRight },
		{ "Jump", Action::Jump },
		{ "MenuUp", Action::MenuUp },
		{ "MenuDown", Action::MenuDown },
		{ "MenuLeft", Action::MenuLeft },
		{ "MenuRight", Action::MenuRight },
		{ "MenuConfirm", Action::MenuConfirm },
		{ "MenuBack", Action::MenuBack }
	};

	const std::unordered_map<std::string, sf::Keyboard::Key> KEY_NAMES =
	{
		{ "A", sf::Keyboard::Key::A }, { "B", sf::Keyboard::Key::B }, { "C", sf::Keyboard::Key::C },
		{ "D", sf::Keyboard::Key::D }, { "E", sf::Keyboard::Key::E }, { "F", sf::Keyboard::Key::F },
		{ "G", sf::Keyboard::Key::G }, { "H", sf::Keyboard::Key::H }, { "I", sf::Keyboard::Key::I },
		{ "J", sf::Keyboard::Key::J }, { "K", sf::Keyboard::Key::K }, { "L", sf::Keyboard::Key::L },
		{ "M", sf::Keyboard::Key::M }, { "N", sf::Keyboard::Key::N }, { "O", sf::Keyboard::Key::O },
		{ "P", sf::Keyboard::Key::P }, { "Q", sf::Keyboard::Key::Q }, { "R", sf::Keyboard::Key::R },
		{ "S", sf::Keyboard::Key::S }, { "T", sf::Keyboard::Key::T }, { "U", sf::Keyboard::Key::U },
		{ "V", sf::Keyboard::Key::V }, { "W", sf::Keyboard::Key::W }, { "X", sf::Keyboard::Key::X },
		{ "Y", sf::Keyboard::Key::Y }, { "Z", sf::Keyboard::Key::Z },
		{ "Num0", sf::Keyboard::Key::Num0 }, { "Num1", sf::Keyboard::Key::Num1 },
		{ "Num2", sf::Keyboard::Key::Num2 }, { "Num3", sf::Keyboard::Key::Num3 },
		{ "Num4", sf::Keyboard::Key::Num4 }, { "Num5", sf::Keyboard::Key::Num5 },
		{ "Num6", sf::Keyboard::Key::Num6 }, { "Num7", sf::Keyboard::Key::Num7 },
		{ "Num8", sf::Keyboard::Key::Num8 }, { "Num9", sf::Keyboard::Key::Num9 },
		{ "Left", sf::Keyboard::Key::Left }, { "Right", sf::Keyboard::Key::Right },
		{ "Up", sf::Keyboard::Key::Up }, { "Down", sf::Keyboard::Key::Down },
		{ "Space", sf::Keyboard::Key::Space }, { "Enter", sf::Keyboard::Key::Enter },
		{ "Escape", sf::Keyboard::Key::Escape }, { "Backspace", sf::Keyboard::Key::Backspace },
		{ "Tab", sf::Keyboard::Key::Tab },
		{ "LShift", sf::Keyboard::Key::LShift }, { "RShift", sf::Keyboard::Key::RShift },
		{ "LControl", sf::Keyboard::Key::LControl }, { "RControl", sf::Keyboard::Key::RControl }
	};

	const std::unordered_map<std::string, sf::Joystick::Axis> AXIS_NAMES =
	{
		{ "X", sf::Joystick::Axis::X }, { "Y", sf::Joystick::Axis::Y },
		{ "Z", sf::Joystick::Axis::Z }, { "R", sf::Joystick::Axis::R },
		{ "U", sf::Joystick::Axis::U }, { "V", sf::Joystick::Axis::V },
		{ "PovX", sf::Joystick::Axis::PovX }, { "PovY", sf::Joystick::Axis::PovY }
	};
}

void Input::LoadConfig(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("Input: cannot open config: " + path);

	const nlohmann::json data = nlohmann::json::parse(file);

	axisThreshold = data.value("axisThreshold", 50.0f);

	for (std::vector<Binding>& list : bindings)
		list.clear();

	for (const auto& [actionName, bindingList] : data.at("bindings").items())
	{
		const auto actionFound = ACTION_NAMES.find(actionName);
		if (actionFound == ACTION_NAMES.end())
			throw std::runtime_error("Input: unknown action '" + actionName + "'");

		std::vector<Binding>& target = bindings[static_cast<int>(actionFound->second)];

		for (const auto& bindingData : bindingList)
		{
			Binding binding;

			if (bindingData.contains("key"))
			{
				const std::string keyName = bindingData.at("key");
				const auto keyFound = KEY_NAMES.find(keyName);
				if (keyFound == KEY_NAMES.end())
					throw std::runtime_error("Input: unknown key '" + keyName + "'");

				binding.type = BindingType::Key;
				binding.key = keyFound->second;
			}
			else if (bindingData.contains("button"))
			{
				binding.type = BindingType::Button;
				binding.button = bindingData.at("button");
			}
			else if (bindingData.contains("axis"))
			{
				const std::string axisName = bindingData.at("axis");
				const auto axisFound = AXIS_NAMES.find(axisName);
				if (axisFound == AXIS_NAMES.end())
					throw std::runtime_error("Input: unknown axis '" + axisName + "'");

				binding.type = BindingType::Axis;
				binding.axis = axisFound->second;
				binding.direction = bindingData.at("direction");
			}
			else
			{
				throw std::runtime_error("Input: binding needs 'key', 'button' or 'axis'");
			}

			target.push_back(binding);
		}
	}
}

int Input::FindGamepad()
{
	for (unsigned int i = 0; i < sf::Joystick::Count; i++)
	{
		if (sf::Joystick::isConnected(i))
			return static_cast<int>(i);
	}

	return -1;
}

bool Input::IsBindingDown(const Binding& binding, int gamepad, bool& fromGamepad) const
{
	switch (binding.type)
	{
	case BindingType::Key:
		fromGamepad = false;
		return sf::Keyboard::isKeyPressed(binding.key);

	case BindingType::Button:
		fromGamepad = true;
		return gamepad >= 0 && sf::Joystick::isButtonPressed(static_cast<unsigned int>(gamepad), binding.button);

	case BindingType::Axis:
		fromGamepad = true;
		if (gamepad < 0)
			return false;
		return sf::Joystick::getAxisPosition(static_cast<unsigned int>(gamepad), binding.axis) * binding.direction > axisThreshold;
	}

	return false;
}

void Input::Update()
{
	const int gamepad = FindGamepad();

	bool anyNewPress = false;
	InputDevice pressDevice = activeDevice;

	for (int i = 0; i < ACTION_COUNT; i++)
	{
		previousDown[i] = currentDown[i];

		bool down = false;
		bool downFromGamepad = false;

		for (const Binding& binding : bindings[i])
		{
			bool fromGamepad = false;
			if (IsBindingDown(binding, gamepad, fromGamepad))
			{
				down = true;
				if (fromGamepad)
					downFromGamepad = true;
			}
		}

		currentDown[i] = down;

		if (down && !previousDown[i])
		{
			anyNewPress = true;
			pressDevice = downFromGamepad ? InputDevice::Gamepad : InputDevice::Keyboard;
		}
	}

	if (anyNewPress)
		activeDevice = pressDevice; // instant switch on the first press of either device
}

bool Input::IsDown(Action action) const
{
	return currentDown[static_cast<int>(action)];
}

bool Input::WasPressed(Action action) const
{
	const int index = static_cast<int>(action);
	return currentDown[index] && !previousDown[index];
}

bool Input::WasReleased(Action action) const
{
	const int index = static_cast<int>(action);
	return !currentDown[index] && previousDown[index];
}

float Input::GetAxisX() const
{
	const float right = IsDown(Action::MoveRight) ? 1.0f : 0.0f;
	const float left = IsDown(Action::MoveLeft) ? 1.0f : 0.0f;
	return right - left;
}