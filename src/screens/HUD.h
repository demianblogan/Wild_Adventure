#pragma once

#include "ui/DataLoader.h"
#include "ui/Root.h"

struct Context;

namespace sf
{
	class RenderTarget;
}

// In-game heads-up display: the score counter, the row of hearts and the
// "Level X" banner that greets a fresh level start. The owning GameState feeds
// it the current score and health each frame; the HUD owns all the presentation
// state (lazy label updates, the heart blink-out, the banner slide animation).
class HUD
{
public:
	HUD(Context& context);

	// Loads data/ui/hud.json. Call after the fonts are loaded.
	void Build(int levelNumber, bool isLevelBannerVisible);

	// The hearts row is sized to the player's maximum health, known only once the
	// player has spawned.
	void SetMaxHearts(int maxHearts);

	// Refreshes the score label, but only when the value actually changed.
	void SetScore(int score);

	// Kicks off the "Level X" banner slide-in (no-op when the banner is disabled).
	void StartBanner();

	void UpdateBanner(float deltaTime);
	void UpdateHearts(int currentHealth, float deltaTime);
	void Update(float deltaTime);

	void Draw(sf::RenderTarget& target);

private:
	void DrawBanner(sf::RenderTarget& target);

	Context& context;

	UI::Root interface;
	UI::DataLoader loader;

	int levelNumber = 1;

	int score = 0;
	int previousScore = -1;

	int lastLocalizationRevision = 0; // detects a language change made mid-level (e.g. via the pause menu)

	int maxHearts = 3;
	int displayedHealth = 3;
	int blinkingHeart = -1;
	float blinkTimer = 0.0f;

	static constexpr float HeartBlinkDuration = 0.5f;

	// While down to the last heart, it double-blinks on a loop instead of
	// just sitting there -- two quick blinks then a longer pause, timed to
	// match Haptics::HeartbeatPulser's vibration/lightbar beat (see
	// core/HapticCues.h) so the heart, the controller buzz and the lightbar
	// all read as the same heartbeat. Duplicated here rather than shared
	// because pulling that header in (and everything it drags in for
	// talking to the controller) just for three float constants isn't worth
	// coupling this purely-visual class to it.
	float criticalHeartbeatTimer = 0.0f;
	float criticalHeartbeatBlinkRemaining = 0.0f;
	bool isCriticalHeartbeatSecondTap = false;

	static constexpr float CriticalHeartbeatTapDuration = 0.05f;
	static constexpr float CriticalHeartbeatGapBetweenTaps = 0.12f;
	static constexpr float CriticalHeartbeatGapAfterPair = 0.55f;

	// "Level X" banner: slides in from above the screen, holds, slides back out.
	enum class BannerPhase
	{
		Hidden,
		SlideIn,
		Hold,
		SlideOut,
		Done
	};

	BannerPhase bannerPhase = BannerPhase::Hidden;
	float bannerTimer = 0.0f;
	bool isLevelBannerVisible = true; // false on checkpoint respawns

	static constexpr float BannerSlideTime = 0.45f; // slide in/out duration
	static constexpr float BannerHoldTime = 2.0f;   // time fully visible
	static constexpr float BannerStartY = -40.0f;   // off-screen above
	static constexpr float BannerTargetY = 90.0f;   // about a third of the screen
};
