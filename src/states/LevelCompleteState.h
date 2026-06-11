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

	// Helpers for rendering individual elements.
	void DrawPanel(sf::RenderTarget& rt) const;
	void DrawTitle(sf::RenderTarget& rt) const;
	void DrawStars(sf::RenderTarget& rt) const;
	void DrawStat(sf::RenderTarget& rt, const std::string& label, float y) const;

	std::string levelPath;
	int levelNumber;
	int deathCount;
	int fruitsCollected;
	int maxFruits;
	int enemiesKilled;
	int maxEnemies;

	// Which stats/stars have been revealed.
	bool showDeaths   = false;
	bool showFruits   = false;
	bool showEnemies  = false;
	bool star1Earned  = false;
	bool star2Earned  = false;
	bool star3Earned  = false;

	// Current animated display values (fractional for smooth counting).
	float displayedDeaths   = 0.0f;
	float displayedFruits   = 0.0f;
	float displayedEnemies  = 0.0f;

	Phase phase     = Phase::Title;
	float phaseTimer = 0.0f;

	// Star sprite animation.
	float starAnimTimer  = 0.0f;
	int   starFrame      = 0;

	static constexpr float STAR_FRAME_DURATION = 0.07f;
	static constexpr int   STAR_FRAME_COUNT    = 13;
	static constexpr int   STAR_FRAME_SIZE     = 32;

	static constexpr float TITLE_WAIT    = 1.0f;
	static constexpr float COUNT_DURATION = 0.7f;
	static constexpr float STAR_PAUSE    = 0.35f;

	UI::Root       completeInterface;
	UI::DataLoader completeLoader;

	NavRequest pendingRequest = NavRequest::None;
};
