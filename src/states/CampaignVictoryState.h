#pragma once

#include "core/State.h"
#include "ui/DataLoader.h"
#include "ui/Root.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/String.hpp>

#include <cstddef>
#include <vector>

// Shown once per campaign after the final level is completed: a black screen
// with a congratulation typed out letter by letter, then a button to the menu.
class CampaignVictoryState : public State
{
public:
	CampaignVictoryState(Context& context);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	enum class Phase
	{
		Typing,   // letters appear one by one
		Waiting,  // full text shown, pause before the button
		Done      // button visible and interactive
	};

	struct Line
	{
		sf::String   text;
		unsigned int characterSize;
		sf::Color    color;
		float        centerY;
	};

	void RegisterActions();
	void SkipToEnd();
	std::size_t TotalCharacters() const;

	std::vector<Line> lines;

	Phase phase            = Phase::Typing;
	float typeTimer        = 0.0f;   // accumulates time toward the next letter
	float waitTimer        = 0.0f;
	std::size_t revealed   = 0;      // letters currently visible across all lines

	bool goToMenu = false;

	static constexpr float CHAR_INTERVAL = 0.03f;
	static constexpr float BUTTON_DELAY  = 2.0f;

	UI::Root       victoryInterface;
	UI::DataLoader victoryLoader;
};
