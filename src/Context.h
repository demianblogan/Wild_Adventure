#pragma once

class VirtualScreen;
class StateMachine;
struct Resources;
class Input;
class Settings;
class GraphicsTarget;
class Campaign;
class LocalizationManager;

namespace Audio
{
	class Mixer;
}

namespace Haptics
{
	class GamepadHaptics;
}

struct Context
{
	VirtualScreen& virtualScreen;
	StateMachine& stateMachine;
	Resources& resources;
	Audio::Mixer& audioMixer;
	Input& input;
	Settings& settings;
	GraphicsTarget& graphics;
	Campaign& campaign;
	LocalizationManager& localization;
	Haptics::GamepadHaptics& gamepadHaptics;
};