#pragma once

class VirtualScreen;
class StateMachine;
class Resources;

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
};