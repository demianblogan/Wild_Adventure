#pragma once

#include "core/State.h"
#include "ui/DataLoader.h"
#include "ui/Root.h"

#include <string>

class LevelCompleteState : public State
{
public:
	LevelCompleteState(Context& context, std::string levelPath, int levelNumber,
		int deathCount, int fruitsCollected, int maxFruits,
		int enemiesKilled, int maxEnemies);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	enum class NavRequest { None, Continue, Restart, QuitToMenu };

	// Animation phases for the sequential stat reveal.
	enum class Phase
	{
		Title,
		CountDeaths,
		StarDeaths,
		CountFruits,
		StarFruits,
		CountEnemies,
		StarEnemies,
		Done
	};

	void RegisterActions();
	void ApplyPendingNavigation();
	void SkipToEnd();

	void AdvancePhase();

	void DrawStars(sf::RenderTarget& rt) const;

	std::string levelPath;
	int levelNumber;
	int deathCount;
	int fruitsCollected;
	int maxFruits;
	int enemiesKilled;
	int maxEnemies;

	// Which stats/stars have been revealed.
	bool hasRevealedDeaths  = false;
	bool hasRevealedFruits  = false;
	bool hasRevealedEnemies = false;
	bool hasEarnedStar1     = false;
	bool hasEarnedStar2     = false;
	bool hasEarnedStar3     = false;

	// Current animated display values (fractional for smooth counting).
	float displayedDeaths   = 0.0f;
	float displayedFruits   = 0.0f;
	float displayedEnemies  = 0.0f;

	Phase phase     = Phase::Title;
	float phaseTimer = 0.0f;

	// Star sprite animation.
	float starAnimTimer  = 0.0f;
	int   starFrame      = 0;

	static constexpr float StarFrameDuration = 0.07f;
	static constexpr int   StarFrameCount    = 13;
	static constexpr int   StarFrameSize     = 32;

	static constexpr float TitleWait    = 1.0f;
	static constexpr float CountDuration = 0.7f;
	static constexpr float StarPause    = 0.35f;

	UI::Root       completeInterface;
	UI::DataLoader completeLoader;

	NavRequest pendingRequest = NavRequest::None;
};
