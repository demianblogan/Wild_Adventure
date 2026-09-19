#include "CampaignVictoryState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Campaign.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "localization/LocalizationManager.h"
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
	const std::string VictoryUiPath = "data/ui/menu/campaign_victory.json";

	constexpr float ScreenWidth = static_cast<float>(VirtualScreen::Width);
	constexpr float ScreenHeight = static_cast<float>(VirtualScreen::Height);
	constexpr float CenterX = ScreenWidth / 2.f;
}

CampaignVictoryState::CampaignVictoryState(Context& context)
	: State(context)
	, victoryInterface(context.virtualScreen)
	, victoryLoader(context.resources)
{
	victoryLoader.SetButtonSounds(context.audioMixer, "ui_hover", "ui_press");
	victoryLoader.SetLocalization(context.localization);
	RegisterActions();
	victoryInterface.SetContent(victoryLoader.LoadFromFile(VictoryUiPath));
	victoryInterface.ResetFocus();

	const sf::Color gold(244, 199, 110, 255);
	const sf::Color white(255, 255, 255, 255);

	// The congratulation, typed out top to bottom. Blank lines are spacing only.
	const auto toUtf8 = [](const std::string& text) { return sf::String::fromUtf8(text.begin(), text.end()); };
	lines = {
		{ toUtf8(context.localization.GetText("campaign_victory.title")),        24, gold,  58.f },
		{ toUtf8(context.localization.GetText("campaign_victory.line_trap")),    16, white, 96.f },
		{ toUtf8(context.localization.GetText("campaign_victory.line_enemy")),   16, white, 116.f },
		{ toUtf8(context.localization.GetText("campaign_victory.complete")),     24, gold,  154.f },
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
	victoryLoader.RegisterAction("cv_menu", [this] { wasMenuRequested = true; });
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

		while (typeTimer >= CharInterval && revealed < TotalCharacters())
		{
			typeTimer -= CharInterval;
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

		if (waitTimer >= ButtonDelay)
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

		if (wasMenuRequested)
		{
			wasMenuRequested = false;
			context.stateMachine.Clear();
			context.stateMachine.Push(std::make_unique<MenuState>(context));
		}
		break;
	}
}

void CampaignVictoryState::Render(float /*interpolationFactor*/)
{
	sf::RenderTarget& rt = context.virtualScreen.GetRenderTarget();
	context.virtualScreen.SetCameraCenter(ScreenWidth / 2.f, ScreenHeight / 2.f);

	// Solid black background.
	sf::RectangleShape background({ ScreenWidth, ScreenHeight });
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
		text.setPosition({ CenterX, line.centerY });

		text.setString(line.text.substring(0, visible));
		text.setFillColor(line.color);
		rt.draw(text);
	}

	if (phase == Phase::Done)
		victoryInterface.Draw(rt);

	// Bloom the highlighted button.
	context.virtualScreen.CompositeGlow(VirtualScreen::GlowUiStrength);
}
