#pragma once

#include "core/State.h"
#include "core/ecs/Registry.h"
#include "systems/AnimationSystem.h"
#include "systems/MovementSystem.h"
#include "systems/RenderSystem.h"

#include <optional>

class TestState : public State
{
public:
	TestState(Context& context);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	Registry registry;
	std::optional<MovementSystem> movementSystem;
	std::optional<AnimationSystem> animationSystem;
	std::optional<RenderSystem> renderSystem;
};