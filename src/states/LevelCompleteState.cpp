#include "LevelCompleteState.h"

#include "Context.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "states/GameState.h"
#include "states/MenuState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

namespace
{
	const std::string LEVEL_COMPLETE_UI_PATH = "data/ui/menu/level_complete.json";

	constexpr float W = static_cast<float>(VirtualScreen::WIDTH);
	constexpr float H = static_cast<float>(VirtualScreen::HEIGHT);

	// Panel geometry (centered on screen).
	constexpr float PANEL_W = 460.f;
	constexpr float PANEL_H = 200.f;
	constexpr float PANEL_X = (W - PANEL_W) / 2.f;
	constexpr float PANEL_Y = (H - PANEL_H) / 2.f;  // 35

	// Vertical positions. All text sizes are multiples of 8 for pixel-sharp rendering.
	constexpr float Y_TITLE   = PANEL_Y + 18.f;   // 53
	constexpr float Y_STARS   = PANEL_Y + 50.f;   // 85  — 28-px stars centered here
	constexpr float Y_DEATHS  = PANEL_Y + 88.f;   // 123
	constexpr float Y_FRUITS  = PANEL_Y + 112.f;  // 147
	constexpr float Y_ENEMIES = PANEL_Y + 136.f;  // 171

	constexpr float CX = W / 2.f;

	void DrawCenteredText(sf::RenderTarget& rt, const sf::Font& font,
		const std::string& str, unsigned int charSize,
		sf::Color fill, sf::Color outline, float outlineThickness,
		float cx, float cy)
	{
		sf::Text text(font, str, charSize);
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
	: State(context, /*rendersStateBelow=*/true, /*updatesStateBelow=*/false)
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
	RegisterActions();
	completeInterface.SetContent(completeLoader.LoadFromFile(LEVEL_COMPLETE_UI_PATH));
	completeInterface.ResetFocus();
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
	if (starAnimTimer >= STAR_FRAME_DURATION)
	{
		starAnimTimer -= STAR_FRAME_DURATION;
		starFrame = (starFrame + 1) % STAR_FRAME_COUNT;
	}

	phaseTimer += deltaTime;

	switch (phase)
	{
	case Phase::Title:
		if (phaseTimer >= TITLE_WAIT)
		{
			showDeaths = true;
			phaseTimer = 0.f;
			phase = Phase::CountDeaths;
		}
		break;

	case Phase::CountDeaths:
	{
		const float t = std::min(phaseTimer / COUNT_DURATION, 1.f);
		displayedDeaths = t * static_cast<float>(deathCount);
		if (phaseTimer >= COUNT_DURATION)
		{
			displayedDeaths = static_cast<float>(deathCount);
			star1Earned = (deathCount == 0);
			phaseTimer = 0.f;
			phase = Phase::StarDeaths;
		}
		break;
	}

	case Phase::StarDeaths:
		if (phaseTimer >= STAR_PAUSE)
		{
			showFruits = true;
			phaseTimer = 0.f;
			phase = Phase::CountFruits;
		}
		break;

	case Phase::CountFruits:
	{
		const float t = std::min(phaseTimer / COUNT_DURATION, 1.f);
		displayedFruits = t * static_cast<float>(fruitsCollected);
		if (phaseTimer >= COUNT_DURATION)
		{
			displayedFruits = static_cast<float>(fruitsCollected);
			star2Earned = (maxFruits > 0 && fruitsCollected >= maxFruits);
			phaseTimer = 0.f;
			phase = Phase::StarFruits;
		}
		break;
	}

	case Phase::StarFruits:
		if (phaseTimer >= STAR_PAUSE)
		{
			showEnemies = true;
			phaseTimer = 0.f;
			phase = Phase::CountEnemies;
		}
		break;

	case Phase::CountEnemies:
	{
		const float t = std::min(phaseTimer / COUNT_DURATION, 1.f);
		displayedEnemies = t * static_cast<float>(enemiesKilled);
		if (phaseTimer >= COUNT_DURATION)
		{
			displayedEnemies = static_cast<float>(enemiesKilled);
			star3Earned = (maxEnemies > 0 && enemiesKilled >= maxEnemies);
			phaseTimer = 0.f;
			phase = Phase::StarEnemies;
		}
		break;
	}

	case Phase::StarEnemies:
		if (phaseTimer >= STAR_PAUSE)
			AdvancePhase();
		break;

	default:
		break;
	}
}

void LevelCompleteState::SkipToEnd()
{
	showDeaths  = true;
	showFruits  = true;
	showEnemies = true;
	displayedDeaths   = static_cast<float>(deathCount);
	displayedFruits   = static_cast<float>(fruitsCollected);
	displayedEnemies  = static_cast<float>(enemiesKilled);
	star1Earned = (deathCount == 0);
	star2Earned = (maxFruits > 0 && fruitsCollected >= maxFruits);
	star3Earned = (maxEnemies > 0 && enemiesKilled >= maxEnemies);
	AdvancePhase();
}

void LevelCompleteState::AdvancePhase()
{
	phase = Phase::Done;
	completeInterface.ResetFocus();
}

void LevelCompleteState::ApplyPendingNavigation()
{
	switch (pendingRequest)
	{
	case NavRequest::Continue:
	case NavRequest::QuitToMenu:
		context.stateMachine.Clear();
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
	context.virtualScreen.SetCameraCenter(W / 2.f, H / 2.f);

	// Dim the level behind the menu.
	sf::RectangleShape overlay({ W, H });
	overlay.setFillColor(sf::Color(0, 0, 0, 150));
	rt.draw(overlay);

	// Dark panel background.
	sf::RectangleShape panel({ PANEL_W, PANEL_H });
	panel.setPosition({ PANEL_X, PANEL_Y });
	panel.setFillColor(sf::Color(18, 12, 38, 220));
	panel.setOutlineColor(sf::Color(80, 55, 120, 200));
	panel.setOutlineThickness(1.5f);
	rt.draw(panel);

	const sf::Font& font = context.resources.fonts.Get("main");
	const sf::Color gold(244, 199, 110, 255);
	const sf::Color white(255, 255, 255, 255);
	const sf::Color outline(58, 42, 77, 255);

	// Title — always visible.
	const std::string titleStr = "Level " + std::to_string(levelNumber) + " Complete!";
	DrawCenteredText(rt, font, titleStr, 16, gold, outline, 1.f, CX, Y_TITLE);

	// Stars row — always visible; filled in as stars are earned.
	DrawStars(rt);

	if (showDeaths)
	{
		const std::string s = "Deaths: " + std::to_string(static_cast<int>(displayedDeaths));
		DrawCenteredText(rt, font, s, 16, white, outline, 1.f, CX, Y_DEATHS);
	}

	if (showFruits)
	{
		const std::string s = "Fruits: " +
			std::to_string(static_cast<int>(displayedFruits)) + "/" + std::to_string(maxFruits);
		DrawCenteredText(rt, font, s, 16, white, outline, 1.f, CX, Y_FRUITS);
	}

	if (showEnemies)
	{
		const std::string s = "Enemies: " +
			std::to_string(static_cast<int>(displayedEnemies)) + "/" + std::to_string(maxEnemies);
		DrawCenteredText(rt, font, s, 16, white, outline, 1.f, CX, Y_ENEMIES);
	}

	if (phase == Phase::Done)
		completeInterface.Draw(rt);
}

void LevelCompleteState::DrawStars(sf::RenderTarget& rt) const
{
	if (!context.resources.textures.Has("star"))
		return;

	const sf::Texture& starTex = context.resources.textures.Get("star");

	constexpr float DISPLAY_SIZE = 28.f;
	constexpr float SPACING      = 44.f;
	const float scale = DISPLAY_SIZE / static_cast<float>(STAR_FRAME_SIZE);

	const bool earned[3] = { star1Earned, star2Earned, star3Earned };

	for (int i = 0; i < 3; ++i)
	{
		const float starCX = CX + static_cast<float>(i - 1) * SPACING;
		const float starX  = starCX - DISPLAY_SIZE / 2.f;
		const float starY  = Y_STARS - DISPLAY_SIZE / 2.f;

		sf::Sprite sprite(starTex);

		if (earned[i])
		{
			sprite.setTextureRect(sf::IntRect(
				{ starFrame * STAR_FRAME_SIZE, 0 },
				{ STAR_FRAME_SIZE, STAR_FRAME_SIZE }));
			sprite.setColor(sf::Color::White);
		}
		else
		{
			sprite.setTextureRect(sf::IntRect(
				{ 0, 0 },
				{ STAR_FRAME_SIZE, STAR_FRAME_SIZE }));
			sprite.setColor(sf::Color(80, 80, 80, 200));
		}

		sprite.setScale({ scale, scale });
		sprite.setPosition({ starX, starY });
		rt.draw(sprite);
	}
}
