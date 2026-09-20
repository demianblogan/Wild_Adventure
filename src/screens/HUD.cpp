#include "HUD.h"

#include "Context.h"
#include "core/Resources.h"
#include "core/VirtualScreen.h"
#include "localization/LocalizationManager.h"
#include "ui/Element.h"
#include "ui/Label.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>

#include <algorithm>
#include <cmath>
#include <string>

HUD::HUD(Context& context)
	: context(context)
	, interface(context.virtualScreen)
	, loader(context.resources)
{
	loader.SetLocalization(context.localization);
}

void HUD::Build(int levelNumber, bool isLevelBannerVisible)
{
	this->levelNumber = levelNumber;
	this->isLevelBannerVisible = isLevelBannerVisible;

	interface.SetContent(loader.LoadFromFile("data/ui/hud.json"));
	lastLocalizationRevision = context.localization.Revision();
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
			label->SetText(context.localization.FormatText("hud.score", "score", std::to_string(score)));
	}

	previousScore = score;
}

void HUD::StartBanner()
{
	if (isLevelBannerVisible)
		bannerPhase = BannerPhase::SlideIn;
}

void HUD::UpdateHearts(int currentHealth, float deltaTime)
{
	// A point was just lost: start blinking the rightmost shown heart.
	if (currentHealth < displayedHealth && blinkingHeart < 0)
	{
		blinkingHeart = currentHealth; // heart index that will disappear
		blinkTimer = HeartBlinkDuration;
		displayedHealth = currentHealth;
	}
	// Health was restored (e.g. touching a checkpoint): refill the hearts at once
	// and cancel any heart still blinking out.
	else if (currentHealth > displayedHealth)
	{
		displayedHealth = currentHealth;
		blinkingHeart = -1;
	}

	bool isBlinkOn = true;
	if (blinkingHeart >= 0)
	{
		blinkTimer -= deltaTime;
		isBlinkOn = std::fmod(blinkTimer, 0.12f) < 0.06f; // fast on/off

		if (blinkTimer <= 0.0f)
			blinkingHeart = -1; // fully gone now
	}

	// The last heart double-blinks on a loop for as long as it's the only one
	// left (and nothing is already blinking it away above).
	const bool isCriticalHeartbeatActive = (displayedHealth == 1 && blinkingHeart < 0);

	if (isCriticalHeartbeatActive)
	{
		criticalHeartbeatTimer -= deltaTime;
		if (criticalHeartbeatBlinkRemaining > 0.0f)
			criticalHeartbeatBlinkRemaining -= deltaTime;

		if (criticalHeartbeatTimer <= 0.0f)
		{
			criticalHeartbeatBlinkRemaining = CriticalHeartbeatTapDuration;
			criticalHeartbeatTimer = isCriticalHeartbeatSecondTap
				? CriticalHeartbeatGapAfterPair
				: CriticalHeartbeatGapBetweenTaps;
			isCriticalHeartbeatSecondTap = !isCriticalHeartbeatSecondTap;
		}
	}
	else
	{
		criticalHeartbeatTimer = 0.0f;
		criticalHeartbeatBlinkRemaining = 0.0f;
		isCriticalHeartbeatSecondTap = false;
	}

	const bool isCriticalHeartHidden = isCriticalHeartbeatActive && criticalHeartbeatBlinkRemaining > 0.0f;

	for (int i = 0; i < maxHearts; i++)
	{
		UI::Element* heart = interface.FindByName("heart" + std::to_string(i));
		if (heart == nullptr)
			continue;

		if (i < displayedHealth)
			heart->isVisible = !(i == 0 && isCriticalHeartHidden); // settled, alive (or mid-heartbeat blink)
		else if (i == blinkingHeart)
			heart->isVisible = isBlinkOn;    // blinking out
		else
			heart->isVisible = false;      // gone
	}
}

void HUD::UpdateBanner(float deltaTime)
{
	if (bannerPhase == BannerPhase::Hidden || bannerPhase == BannerPhase::Done)
		return;

	bannerTimer += deltaTime;

	if (bannerPhase == BannerPhase::SlideIn && bannerTimer >= BannerSlideTime)
	{
		bannerPhase = BannerPhase::Hold;
		bannerTimer = 0.0f;
	}
	else if (bannerPhase == BannerPhase::Hold && bannerTimer >= BannerHoldTime)
	{
		bannerPhase = BannerPhase::SlideOut;
		bannerTimer = 0.0f;
	}
	else if (bannerPhase == BannerPhase::SlideOut && bannerTimer >= BannerSlideTime)
	{
		bannerPhase = BannerPhase::Done;
	}
}

void HUD::Update(float deltaTime)
{
	if (context.localization.Revision() != lastLocalizationRevision)
	{
		lastLocalizationRevision = context.localization.Revision();

		if (auto* label = dynamic_cast<UI::Label*>(interface.FindByName("health_label")))
			label->SetText(context.localization.GetText("hud.health"));

		// SetScore only re-resolves "score" when the number itself changes
		// (see previousScore below), so a language change alone would
		// otherwise sit stale until the score next ticks up.
		if (auto* label = dynamic_cast<UI::Label*>(interface.FindByName("score")))
			label->SetText(context.localization.FormatText("hud.score", "score", std::to_string(score)));
	}

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

	float y = BannerTargetY;

	if (bannerPhase == BannerPhase::SlideIn)
	{
		// Ease out: fast entrance that settles softly.
		const float t = std::min(bannerTimer / BannerSlideTime, 1.0f);
		const float eased = 1.0f - (1.0f - t) * (1.0f - t);
		y = BannerStartY + (BannerTargetY - BannerStartY) * eased;
	}
	else if (bannerPhase == BannerPhase::SlideOut)
	{
		// Ease in: slow start, accelerating off the screen.
		const float t = std::min(bannerTimer / BannerSlideTime, 1.0f);
		const float eased = t * t;
		y = BannerTargetY + (BannerStartY - BannerTargetY) * eased;
	}

	const std::string bannerText = context.localization.FormatText("hud.level_banner", "level", std::to_string(levelNumber));
	sf::Text text(context.resources.fonts.Get("main"), sf::String::fromUtf8(bannerText.begin(), bannerText.end()), 24);
	text.setFillColor(sf::Color(244, 199, 110));     // warm gold, as on the complete menu
	text.setOutlineColor(sf::Color(58, 42, 77));     // deep purple outline
	text.setOutlineThickness(2.0f);

	const sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin({
		bounds.position.x + bounds.size.x / 2.0f,
		bounds.position.y + bounds.size.y / 2.0f });
	text.setPosition({ std::floor(VirtualScreen::Width / 2.0f), std::floor(y) });

	target.draw(text);
}
