#pragma once

class VirtualScreen;
class StateMachine;
struct Resources;
class Input;
class Settings;
class GraphicsTarget;
class Campaign;

namespace Audio
{
	class Mixer;
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
};