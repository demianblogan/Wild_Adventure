#pragma once

class VirtualScreen;
class StateMachine;
class Resources;
class Input;

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
};