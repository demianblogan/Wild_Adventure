#include "HUD.h"

#include "Context.h"
#include "core/Resources.h"
#include "core/VirtualScreen.h"
#include "ui/Element.h"
#include "ui/Label.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <algorithm>
#include <cmath>
#include <string>

HUD::HUD(Context& context)
	: context(context)
	, interface(context.virtualScreen)
	, loader(context.resources)
{}

void HUD::Build(int levelNumber, bool showLevelBanner)
{
	this->levelNumber = levelNumber;
	this->showLevelBanner = showLevelBanner;

	interface.SetContent(loader.LoadFromFile("data/ui/hud.json"));
}

void HUD::SetMaxHearts(int maxHearts)
{
	this->maxHearts = maxHearts;
	displayedHealth = maxHearts;
}

void HUD::SetScore(int score)
{
	if (score == previousScore)
		return;

	this->score = score;

	if (UI::Element* element = interface.FindByName("score"))
	{
		if (auto* label = dynamic_cast<UI::Label*>(element))
			label->SetText("Score: " + std::to_string(score));
	}

	previousScore = score;
}

void HUD::StartBanner()
{
	if (showLevelBanner)
		bannerPhase = BannerPhase::SlideIn;
}

void HUD::UpdateHearts(int currentHealth, float deltaTime)
{
	// A point was just lost: start blinking the rightmost shown heart.
	if (currentHealth < displayedHealth && blinkingHeart < 0)
	{
		blinkingHeart = currentHealth; // heart index that will disappear
		blinkTimer = HEART_BLINK_DURATION;
		displayedHealth = currentHealth;
	}
	// Health was restored (e.g. touching a checkpoint): refill the hearts at once
	// and cancel any heart still blinking out.
	else if (currentHealth > displayedHealth)
	{
		displayedHealth = currentHealth;
		blinkingHeart = -1;
	}

	bool blinkOn = true;
	if (blinkingHeart >= 0)
	{
		blinkTimer -= deltaTime;
		blinkOn = std::fmod(blinkTimer, 0.12f) < 0.06f; // fast on/off

		if (blinkTimer <= 0.0f)
			blinkingHeart = -1; // fully gone now
	}

	for (int i = 0; i < maxHearts; i++)
	{
		UI::Element* heart = interface.FindByName("heart" + std::to_string(i));
		if (heart == nullptr)
			continue;

		if (i < displayedHealth)
			heart->isVisible = true;       // settled, alive
		else if (i == blinkingHeart)
			heart->isVisible = blinkOn;    // blinking out
		else
			heart->isVisible = false;      // gone
	}
}

void HUD::UpdateBanner(float deltaTime)
{
	if (bannerPhase == BannerPhase::Hidden || bannerPhase == BannerPhase::Done)
		return;

	bannerTimer += deltaTime;

	if (bannerPhase == BannerPhase::SlideIn && bannerTimer >= BANNER_SLIDE_TIME)
	{
		bannerPhase = BannerPhase::Hold;
		bannerTimer = 0.0f;
	}
	else if (bannerPhase == BannerPhase::Hold && bannerTimer >= BANNER_HOLD_TIME)
	{
		bannerPhase = BannerPhase::SlideOut;
		bannerTimer = 0.0f;
	}
	else if (bannerPhase == BannerPhase::SlideOut && bannerTimer >= BANNER_SLIDE_TIME)
	{
		bannerPhase = BannerPhase::Done;
	}
}

void HUD::Update(float deltaTime)
{
	interface.Update(deltaTime);
}

void HUD::Draw(sf::RenderTarget& target)
{
	interface.Draw(target);
	DrawBanner(target);
}

void HUD::DrawBanner(sf::RenderTarget& target)
{
	if (bannerPhase == BannerPhase::Hidden || bannerPhase == BannerPhase::Done)
		return;

	float y = BANNER_TARGET_Y;

	if (bannerPhase == BannerPhase::SlideIn)
	{
		// Ease out: fast entrance that settles softly.
		const float t = std::min(bannerTimer / BANNER_SLIDE_TIME, 1.0f);
		const float eased = 1.0f - (1.0f - t) * (1.0f - t);
		y = BANNER_START_Y + (BANNER_TARGET_Y - BANNER_START_Y) * eased;
	}
	else if (bannerPhase == BannerPhase::SlideOut)
	{
		// Ease in: slow start, accelerating off the screen.
		const float t = std::min(bannerTimer / BANNER_SLIDE_TIME, 1.0f);
		const float eased = t * t;
		y = BANNER_TARGET_Y + (BANNER_START_Y - BANNER_TARGET_Y) * eased;
	}

	sf::Text text(context.resources.fonts.Get("main"), "Level " + std::to_string(levelNumber), 24);
	text.setFillColor(sf::Color(244, 199, 110));     // warm gold, as on the complete menu
	text.setOutlineColor(sf::Color(58, 42, 77));     // deep purple outline
	text.setOutlineThickness(2.0f);

	const sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin({
		bounds.position.x + bounds.size.x / 2.0f,
		bounds.position.y + bounds.size.y / 2.0f });
	text.setPosition({ std::floor(VirtualScreen::WIDTH / 2.0f), std::floor(y) });

	target.draw(text);
}
