#include "Input.h"

#include <nlohmann/json.hpp>

#include <algorithm>
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
		{ "Pause", Action::Pause },
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

	std::string ActionToString(Action action)
	{
		for (const auto& [name, value] : ACTION_NAMES)
			if (value == action)
				return name;

		return "";
	}

	std::string AxisToString(sf::Joystick::Axis axis)
	{
		for (const auto& [name, value] : AXIS_NAMES)
			if (value == axis)
				return name;

		return "X";
	}
}

std::string Input::KeyName(sf::Keyboard::Key key)
{
	for (const auto& [name, value] : KEY_NAMES)
		if (value == key)
			return name;

	return "None";
}

void Input::LoadBindingsFile(const std::string& path, BindingSet target, float* outAxisThreshold)
{
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("Input: cannot open config: " + path);

	const nlohmann::json data = nlohmann::json::parse(file);

	if (outAxisThreshold != nullptr)
		*outAxisThreshold = data.value("axisThreshold", 50.0f);

	for (int i = 0; i < ACTION_COUNT; i++)
		target[i].clear();

	for (const auto& [actionName, bindingList] : data.at("bindings").items())
	{
		const auto actionFound = ACTION_NAMES.find(actionName);
		if (actionFound == ACTION_NAMES.end())
			throw std::runtime_error("Input: unknown action '" + actionName + "'");

		std::vector<Binding>& list = target[static_cast<int>(actionFound->second)];

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

			list.push_back(binding);
		}
	}

	// Escape is reserved: it always pauses the game and backs out of menus,
	// and is not rebindable. Normalize every loaded set (older saved configs
	// may carry a different pause key).
	Binding escape;
	escape.type = BindingType::Key;
	escape.key = sf::Keyboard::Key::Escape;

	std::vector<Binding>& pause = target[static_cast<int>(Action::Pause)];
	std::erase_if(pause, [](const Binding& binding) { return binding.type == BindingType::Key; });
	pause.insert(pause.begin(), escape);

	std::vector<Binding>& back = target[static_cast<int>(Action::MenuBack)];
	if (std::find(back.begin(), back.end(), escape) == back.end())
		back.push_back(escape);
}

void Input::LoadConfig(const std::string& path)
{
	LoadBindingsFile(path, bindings, &axisThreshold);

	for (int i = 0; i < ACTION_COUNT; i++)
		savedBindings[i] = bindings[i];
}

void Input::LoadDefaults(const std::string& path)
{
	LoadBindingsFile(path, defaultBindings, nullptr);
}

bool Input::IsDirty() const
{
	for (int i = 0; i < ACTION_COUNT; i++)
		if (bindings[i] != savedBindings[i])
			return true;

	return false;
}

void Input::Revert()
{
	for (int i = 0; i < ACTION_COUNT; i++)
		bindings[i] = savedBindings[i];
}

void Input::ResetToDefaults()
{
	for (int i = 0; i < ACTION_COUNT; i++)
		bindings[i] = defaultBindings[i];
}

void Input::SaveConfig(const std::string& path)
{
	nlohmann::json data;
	data["axisThreshold"] = axisThreshold;

	nlohmann::json bindingsJSON = nlohmann::json::object();

	for (int i = 0; i < ACTION_COUNT; i++)
	{
		nlohmann::json list = nlohmann::json::array();

		for (const Binding& binding : bindings[i])
		{
			nlohmann::json entry = nlohmann::json::object();

			switch (binding.type)
			{
			case BindingType::Key:
				entry["key"] = KeyName(binding.key);
				break;
			case BindingType::Button:
				entry["button"] = binding.button;
				break;
			case BindingType::Axis:
				entry["axis"] = AxisToString(binding.axis);
				entry["direction"] = binding.direction;
				break;
			}

			list.push_back(entry);
		}

		bindingsJSON[ActionToString(static_cast<Action>(i))] = list;
	}

	data["bindings"] = bindingsJSON;

	std::ofstream file(path);
	if (!file.is_open())
		throw std::runtime_error("Input: cannot write '" + path + "'");

	file << data.dump(1, '\t');

	for (int i = 0; i < ACTION_COUNT; i++)
		savedBindings[i] = bindings[i];
}

sf::Keyboard::Key Input::GetPrimaryKey(Action action) const
{
	for (const Binding& binding : bindings[static_cast<int>(action)])
		if (binding.type == BindingType::Key)
			return binding.key;

	return sf::Keyboard::Key::Unknown;
}

void Input::SetPrimaryKey(Action action, sf::Keyboard::Key key)
{
	std::vector<Binding>& list = bindings[static_cast<int>(action)];

	for (Binding& binding : list)
	{
		if (binding.type == BindingType::Key)
		{
			binding.key = key;
			return;
		}
	}

	Binding binding;
	binding.type = BindingType::Key;
	binding.key = key;
	list.insert(list.begin(), binding);
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