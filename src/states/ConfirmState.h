#pragma once

#include "core/State.h"
#include "ui/DataLoader.h"
#include "ui/Root.h"

#include <functional>
#include <string>

class ConfirmState : public State
{
public:
	ConfirmState(Context& context, const std::string& title, const std::string& message,
		std::function<void()> onYes, std::function<void()> onNo);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	void Close();

	UI::Root dialog;
	UI::DataLoader loader;

	std::function<void()> onYes;
	std::function<void()> onNo;
	bool closed = false;
};