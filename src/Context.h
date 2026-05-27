#pragma once

class VirtualScreen;
class StateMachine;
class Resources;

struct Context
{
	VirtualScreen& virtualScreen;
	StateMachine& stateMachine;
	Resources& resources;
};