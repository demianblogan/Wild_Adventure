#pragma once

class Registry;

class AnimationSystem
{
public:
	AnimationSystem(Registry& registry);

	void Update(float deltaTime);

private:
	Registry& registry;
};