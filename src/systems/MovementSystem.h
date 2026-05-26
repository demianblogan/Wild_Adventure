#pragma once

class Registry;

class MovementSystem
{
public:
	MovementSystem(Registry& registry);

	void Update(float deltaTime);

private:
	Registry& registry;
};