#pragma once

#include "core/State.h"
#include "graphics/Transition.h"
#include "screens/MenuBackdrop.h"
#include "ui/DataLoader.h"
#include "ui/Root.h"

class SplashState : public State
{
public:
	SplashState(Context& context);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	void BuildInterface();

	MenuBackdrop backdrop;

	UI::Root userInterface;
	UI::DataLoader interfaceLoader;

	Transition transition;

	bool isLeaving = false;
};