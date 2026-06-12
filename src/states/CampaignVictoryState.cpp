#include "CampaignVictoryState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Campaign.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "states/MenuState.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <algorithm>
#include <memory>
#include <string>

namespace
{
	const std::string VICTORY_UI_PATH = "data/ui/menu/campaign_victory.json";

	constexpr float W = static_cast<float>(VirtualScreen::WIDTH);
	constexpr float H = static_cast<float>(VirtualScreen::HEIGHT);
	constexpr float CX = W / 2.f;
}

CampaignVictoryState::CampaignVictoryState(Context& context)
	: State(context)
	, victoryInterface(context.virtualScreen)
	, victoryLoader(context.resources)
{
	victoryLoader.SetButtonSounds(context.audioMixer, "ui_hover", "ui_press");
	RegisterActions();
	victoryInterface.SetContent(victoryLoader.LoadFromFile(VICTORY_UI_PATH));
	victoryInterface.ResetFocus();

	const sf::Color gold(244, 199, 110, 255);
	const sf::Color white(255, 255, 255, 255);

	// The congratulation, typed out top to bottom. Blank lines are spacing only.
	lines = {
		{ "Victory!",                     24, gold,  58.f },
		{ "No trap could stop you.",      16, white, 96.f },
		{ "No enemy made you surrender.", 16, white, 116.f },
		{ "Campaign complete!",           24, gold,  154.f },
	};

	// The campaign is over: silence the level music and play the victory jingle.
	context.audioMixer.StopMusic();
	context.audioMixer.PlaySound("campaign_victory");

	// Drop the last level's color grading so the black screen stays neutral.
	context.virtualScreen.SetColorGrading({});

	// Persist immediately so the screen appears only once per campaign.
	context.campaign.MarkVictoryShown();
}

void CampaignVictoryState::RegisterActions()
{
	victoryLoader.RegisterAction("cv_menu", [this] { goToMenu = true; });
}

std::size_t CampaignVictoryState::TotalCharacters() const
{
	std::size_t total = 0;

	for (const Line& line : lines)
		total += line.text.getSize();

	return total;
}

void CampaignVictoryState::HandleEvent(const sf::Event& event)
{
	if (phase == Phase::Done)
		victoryInterface.HandleEvent(event);
}

void CampaignVictoryState::SkipToEnd()
{
	revealed = TotalCharacters();
	phase = Phase::Done;
	victoryInterface.ResetFocus();
}

void CampaignVictoryState::Update(float deltaTime)
{
	Input& input = context.input;

	switch (phase)
	{
	case Phase::Typing:
	{
		// A confirm/back press skips the animation entirely.
		if (input.WasPressed(Action::MenuConfirm) || input.WasPressed(Action::MenuBack))
		{
			SkipToEnd();
			return;
		}

		typeTimer += deltaTime;

		while (typeTimer >= CHAR_INTERVAL && revealed < TotalCharacters())
		{
			typeTimer -= CHAR_INTERVAL;
			revealed++;
		}

		if (revealed >= TotalCharacters())
		{
			waitTimer = 0.f;
			phase = Phase::Waiting;
		}
		break;
	}

	case Phase::Waiting:
		if (input.WasPressed(Action::MenuConfirm) || input.WasPressed(Action::MenuBack))
		{
			SkipToEnd();
			return;
		}

		waitTimer += deltaTime;

		if (waitTimer >= BUTTON_DELAY)
		{
			phase = Phase::Done;
			victoryInterface.ResetFocus();
		}
		break;

	case Phase::Done:
		victoryInterface.Update(deltaTime);

		if (input.WasPressed(Action::MenuConfirm))
			victoryInterface.Confirm(true);
		else if (input.WasReleased(Action::MenuConfirm))
			victoryInterface.Confirm(false);

		if (goToMenu)
		{
			goToMenu = false;
			context.stateMachine.Clear();
			context.stateMachine.Push(std::make_unique<MenuState>(context));
		}
		break;
	}
}

void CampaignVictoryState::Render(float /*interpolationFactor*/)
{
	sf::RenderTarget& rt = context.virtualScreen.GetRenderTarget();
	context.virtualScreen.SetCameraCenter(W / 2.f, H / 2.f);

	// Solid black background.
	sf::RectangleShape background({ W, H });
	background.setFillColor(sf::Color::Black);
	rt.draw(background);

	const sf::Font& font = context.resources.fonts.Get("main");

	// Type out the lines top to bottom. Each line is anchored where its full
	// text will sit, so letters don't shift while they appear.
	std::size_t remaining = revealed;

	for (const Line& line : lines)
	{
		const std::size_t visible = std::min<std::size_t>(remaining, line.text.getSize());
		remaining -= visible;

		if (visible == 0)
			break;

		sf::Text text(font, line.text, line.characterSize);

		const sf::FloatRect fullBounds = text.getLocalBounds();
		text.setOrigin({
			fullBounds.position.x + fullBounds.size.x / 2.f,
			fullBounds.position.y + fullBounds.size.y / 2.f });
		text.setPosition({ CX, line.centerY });

		text.setString(line.text.substring(0, visible));
		text.setFillColor(line.color);
		rt.draw(text);
	}

	if (phase == Phase::Done)
		victoryInterface.Draw(rt);
}
