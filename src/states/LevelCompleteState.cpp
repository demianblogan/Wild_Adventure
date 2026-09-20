#include "LevelCompleteState.h"

#include "Context.h"
#include "core/Campaign.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "localization/LocalizationManager.h"
#include "states/CampaignVictoryState.h"
#include "states/GameState.h"
#include "states/MenuState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/String.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

namespace
{
	const std::string LevelCompleteUiPath = "data/ui/menu/level_complete.json";

	constexpr float ScreenWidth = static_cast<float>(VirtualScreen::Width);
	constexpr float ScreenHeight = static_cast<float>(VirtualScreen::Height);

	// Panel geometry (centered on screen).
	constexpr float PanelW = 460.f;
	constexpr float PanelH = 200.f;
	constexpr float PanelX = (ScreenWidth - PanelW) / 2.f;
	constexpr float PanelY = (ScreenHeight - PanelH) / 2.f;  // 35

	// Vertical positions. All text sizes are multiples of 8 for pixel-sharp rendering.
	constexpr float YTitle   = PanelY + 18.f;   // 53
	constexpr float YStars   = PanelY + 50.f;   // 85  — 28-px stars centered here
	constexpr float YDeaths  = PanelY + 88.f;   // 123
	constexpr float YFruits  = PanelY + 112.f;  // 147
	constexpr float YEnemies = PanelY + 136.f;  // 171

	constexpr float CenterX = ScreenWidth / 2.f;

	void DrawCenteredText(sf::RenderTarget& rt, const sf::Font& font,
		const std::string& str, unsigned int charSize,
		sf::Color fill, sf::Color outline, float outlineThickness,
		float cx, float cy)
	{
		sf::Text text(font, sf::String::fromUtf8(str.begin(), str.end()), charSize);
		text.setFillColor(fill);
		text.setOutlineColor(outline);
		text.setOutlineThickness(outlineThickness);

		const sf::FloatRect bounds = text.getLocalBounds();
		text.setOrigin({
			bounds.position.x + bounds.size.x / 2.f,
			bounds.position.y + bounds.size.y / 2.f });
		text.setPosition({ cx, cy });

		rt.draw(text);
	}
}

LevelCompleteState::LevelCompleteState(Context& context, std::string levelPath, int levelNumber,
	int deathCount, int fruitsCollected, int maxFruits,
	int enemiesKilled, int maxEnemies)
	: State(context, /*isRenderingStateBelow=*/true, /*isUpdatingStateBelow=*/false)
	, completeInterface(context.virtualScreen)
	, completeLoader(context.resources)
	, levelPath(std::move(levelPath))
	, levelNumber(levelNumber)
	, deathCount(deathCount)
	, fruitsCollected(fruitsCollected)
	, maxFruits(maxFruits)
	, enemiesKilled(enemiesKilled)
	, maxEnemies(maxEnemies)
{
	completeLoader.SetButtonSounds(context.audioMixer, "ui_hover", "ui_press");
	completeLoader.SetButtonHaptics(context.gamepadHaptics);
	completeLoader.SetLocalization(context.localization);
	RegisterActions();
	completeInterface.SetContent(completeLoader.LoadFromFile(LevelCompleteUiPath));
	completeInterface.ResetFocus();

	// Persist campaign progress as soon as the menu appears, using the same star
	// rules the reveal animation applies later.
	const int earnedStars =
		(this->deathCount == 0 ? 1 : 0)
		+ ((this->maxFruits > 0 && this->fruitsCollected >= this->maxFruits) ? 1 : 0)
		+ ((this->maxEnemies > 0 && this->enemiesKilled >= this->maxEnemies) ? 1 : 0);

	context.campaign.RecordCompletion(this->levelNumber, earnedStars);
}

void LevelCompleteState::RegisterActions()
{
	completeLoader.RegisterAction("lc_continue", [this] { pendingRequest = NavRequest::Continue; });
	completeLoader.RegisterAction("lc_restart",  [this] { pendingRequest = NavRequest::Restart; });
	completeLoader.RegisterAction("lc_menu",     [this] { pendingRequest = NavRequest::QuitToMenu; });
}

void LevelCompleteState::HandleEvent(const sf::Event& event)
{
	if (phase == Phase::Done)
		completeInterface.HandleEvent(event);
}

void LevelCompleteState::Update(float deltaTime)
{
	Input& input = context.input;

	// Any confirm/back press during animation skips straight to Done.
	if (phase != Phase::Done &&
		(input.WasPressed(Action::MenuConfirm) || input.WasPressed(Action::MenuBack)))
	{
		SkipToEnd();
		return;
	}

	if (phase == Phase::Done)
	{
		completeInterface.Update(deltaTime);

		if (input.WasPressed(Action::MenuLeft) || input.WasPressed(Action::MenuUp))
			completeInterface.NavigateUp();
		else if (input.WasPressed(Action::MenuRight) || input.WasPressed(Action::MenuDown))
			completeInterface.NavigateDown();

		if (input.WasPressed(Action::MenuConfirm))
			completeInterface.Confirm(true);
		else if (input.WasReleased(Action::MenuConfirm))
			completeInterface.Confirm(false);

		ApplyPendingNavigation();
		return;
	}

	// Animate the star sprite continuously.
	starAnimTimer += deltaTime;
	if (starAnimTimer >= StarFrameDuration)
	{
		starAnimTimer -= StarFrameDuration;
		starFrame = (starFrame + 1) % StarFrameCount;
	}

	phaseTimer += deltaTime;

	switch (phase)
	{
	case Phase::Title:
		if (phaseTimer >= TitleWait)
		{
			hasRevealedDeaths = true;
			phaseTimer = 0.f;
			phase = Phase::CountDeaths;
		}
		break;

	case Phase::CountDeaths:
	{
		const float t = std::min(phaseTimer / CountDuration, 1.f);
		displayedDeaths = t * static_cast<float>(deathCount);
		if (phaseTimer >= CountDuration)
		{
			displayedDeaths = static_cast<float>(deathCount);
			hasEarnedStar1 = (deathCount == 0);
			phaseTimer = 0.f;
			phase = Phase::StarDeaths;
		}
		break;
	}

	case Phase::StarDeaths:
		if (phaseTimer >= StarPause)
		{
			hasRevealedFruits = true;
			phaseTimer = 0.f;
			phase = Phase::CountFruits;
		}
		break;

	case Phase::CountFruits:
	{
		const float t = std::min(phaseTimer / CountDuration, 1.f);
		displayedFruits = t * static_cast<float>(fruitsCollected);
		if (phaseTimer >= CountDuration)
		{
			displayedFruits = static_cast<float>(fruitsCollected);
			hasEarnedStar2 = (maxFruits > 0 && fruitsCollected >= maxFruits);
			phaseTimer = 0.f;
			phase = Phase::StarFruits;
		}
		break;
	}

	case Phase::StarFruits:
		if (phaseTimer >= StarPause)
		{
			hasRevealedEnemies = true;
			phaseTimer = 0.f;
			phase = Phase::CountEnemies;
		}
		break;

	case Phase::CountEnemies:
	{
		const float t = std::min(phaseTimer / CountDuration, 1.f);
		displayedEnemies = t * static_cast<float>(enemiesKilled);
		if (phaseTimer >= CountDuration)
		{
			displayedEnemies = static_cast<float>(enemiesKilled);
			hasEarnedStar3 = (maxEnemies > 0 && enemiesKilled >= maxEnemies);
			phaseTimer = 0.f;
			phase = Phase::StarEnemies;
		}
		break;
	}

	case Phase::StarEnemies:
		if (phaseTimer >= StarPause)
			AdvancePhase();
		break;

	default:
		break;
	}
}

void LevelCompleteState::SkipToEnd()
{
	hasRevealedDeaths  = true;
	hasRevealedFruits  = true;
	hasRevealedEnemies = true;
	displayedDeaths   = static_cast<float>(deathCount);
	displayedFruits   = static_cast<float>(fruitsCollected);
	displayedEnemies  = static_cast<float>(enemiesKilled);
	hasEarnedStar1 = (deathCount == 0);
	hasEarnedStar2 = (maxFruits > 0 && fruitsCollected >= maxFruits);
	hasEarnedStar3 = (maxEnemies > 0 && enemiesKilled >= maxEnemies);
	AdvancePhase();
}

void LevelCompleteState::AdvancePhase()
{
	phase = Phase::Done;
	completeInterface.ResetFocus();
}

void LevelCompleteState::ApplyPendingNavigation()
{
	// Leaving the last campaign level forward shows the one-time victory screen.
	const bool showVictory = Campaign::IsLastLevel(levelNumber)
		&& !context.campaign.WasVictoryShown();

	switch (pendingRequest)
	{
	case NavRequest::Continue:
	{
		// Continue goes to the next campaign level; past the last one — to the menu.
		const int nextLevel = levelNumber + 1;

		context.stateMachine.Clear();

		if (Campaign::LevelExists(nextLevel))
			context.stateMachine.Push(std::make_unique<GameState>(context, Campaign::LevelPath(nextLevel), nextLevel));
		else if (showVictory)
			context.stateMachine.Push(std::make_unique<CampaignVictoryState>(context));
		else
			context.stateMachine.Push(std::make_unique<MenuState>(context));
		break;
	}

	case NavRequest::QuitToMenu:
		context.stateMachine.Clear();

		if (showVictory)
			context.stateMachine.Push(std::make_unique<CampaignVictoryState>(context));
		else
			context.stateMachine.Push(std::make_unique<MenuState>(context));
		break;

	case NavRequest::Restart:
		context.stateMachine.Clear();
		context.stateMachine.Push(std::make_unique<GameState>(context, levelPath, levelNumber));
		break;

	case NavRequest::None:
		break;
	}

	pendingRequest = NavRequest::None;
}

void LevelCompleteState::Render(float /*interpolationFactor*/)
{
	sf::RenderTarget& rt = context.virtualScreen.GetRenderTarget();
	context.virtualScreen.SetCameraCenter(ScreenWidth / 2.f, ScreenHeight / 2.f);

	// Dim the level behind the menu.
	sf::RectangleShape overlay({ ScreenWidth, ScreenHeight });
	overlay.setFillColor(sf::Color(0, 0, 0, 150));
	rt.draw(overlay);

	// Dark panel background.
	sf::RectangleShape panel({ PanelW, PanelH });
	panel.setPosition({ PanelX, PanelY });
	panel.setFillColor(sf::Color(18, 12, 38, 220));
	panel.setOutlineColor(sf::Color(80, 55, 120, 200));
	panel.setOutlineThickness(1.5f);
	rt.draw(panel);

	const sf::Font& font = context.resources.fonts.Get("main");
	const sf::Color gold(244, 199, 110, 255);
	const sf::Color white(255, 255, 255, 255);
	const sf::Color outline(58, 42, 77, 255);

	// Title — always visible.
	const std::string titleStr = context.localization.FormatText("level_complete.title", "level", std::to_string(levelNumber));
	DrawCenteredText(rt, font, titleStr, 16, gold, outline, 1.f, CenterX, YTitle);

	// Stars row — always visible; filled in as stars are earned.
	DrawStars(rt);

	if (hasRevealedDeaths)
	{
		const std::string s = context.localization.FormatText("level_complete.deaths", "count", std::to_string(static_cast<int>(displayedDeaths)));
		DrawCenteredText(rt, font, s, 16, white, outline, 1.f, CenterX, YDeaths);
	}

	if (hasRevealedFruits)
	{
		const std::string value = std::to_string(static_cast<int>(displayedFruits)) + "/" + std::to_string(maxFruits);
		const std::string s = context.localization.FormatText("level_complete.fruits", "value", value);
		DrawCenteredText(rt, font, s, 16, white, outline, 1.f, CenterX, YFruits);
	}

	if (hasRevealedEnemies)
	{
		const std::string value = std::to_string(static_cast<int>(displayedEnemies)) + "/" + std::to_string(maxEnemies);
		const std::string s = context.localization.FormatText("level_complete.enemies", "value", value);
		DrawCenteredText(rt, font, s, 16, white, outline, 1.f, CenterX, YEnemies);
	}

	// Bloom the earned stars at world strength, then the highlighted button
	// at UI strength — separate composites so each gets its own intensity.
	context.virtualScreen.CompositeGlow();

	if (phase == Phase::Done)
		completeInterface.Draw(rt);

	context.virtualScreen.CompositeGlow(VirtualScreen::GlowUiStrength);
}

void LevelCompleteState::DrawStars(sf::RenderTarget& rt) const
{
	if (!context.resources.textures.Has("star"))
		return;

	const sf::Texture& starTex = context.resources.textures.Get("star");

	constexpr float DisplaySize = 28.f;
	constexpr float Spacing      = 44.f;
	const float scale = DisplaySize / static_cast<float>(StarFrameSize);

	const bool earned[3] = { hasEarnedStar1, hasEarnedStar2, hasEarnedStar3 };

	for (int i = 0; i < 3; ++i)
	{
		const float starCX = CenterX + static_cast<float>(i - 1) * Spacing;
		const float starX  = starCX - DisplaySize / 2.f;
		const float starY  = YStars - DisplaySize / 2.f;

		sf::Sprite sprite(starTex);

		if (earned[i])
		{
			sprite.setTextureRect(sf::IntRect(
				{ starFrame * StarFrameSize, 0 },
				{ StarFrameSize, StarFrameSize }));
			sprite.setColor(sf::Color::White);
		}
		else
		{
			sprite.setTextureRect(sf::IntRect(
				{ 0, 0 },
				{ StarFrameSize, StarFrameSize }));
			sprite.setColor(sf::Color(80, 80, 80, 200));
		}

		sprite.setScale({ scale, scale });
		sprite.setPosition({ starX, starY });
		rt.draw(sprite);

		if (earned[i])
			context.virtualScreen.GetGlowTarget().draw(sprite);
	}
}
