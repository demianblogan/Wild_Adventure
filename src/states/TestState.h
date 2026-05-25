#pragma once

#include "core/State.h"

class TestState : public State
{
public:
	TestState(Context& context);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;
};