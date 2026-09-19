#pragma once

#include "core/Campaign.h"

#include <array>
#include <functional>

struct Context;

namespace sf
{
	class Event;
	class RenderTarget;
}

// Self-contained "Select Level" grid shown over the live main menu. The owner
// calls Open(), then forwards events / update / render while the grid is shown;
// when the user backs out (or launches a level) WasCloseRequested() turns true.
class SelectLevelController
{
public:
	SelectLevelController(Context& context);

	void Open();
	bool WasCloseRequested() const { return wasCloseRequested; }

	// Called with the level number instead of launching it directly; the
	// owner routes the launch through the character select screen.
	void SetLaunchHandler(std::function<void(int)> handler) { launchHandler = std::move(handler); }

	void HandleEvent(const sf::Event& event);
	void Update(float deltaTime);
	void Render(sf::RenderTarget& target);

private:
	struct Cell
	{
		bool isSelectable = false; // unlocked and its .tmj file exists
		bool isCompleted = false;
		int stars = 0;           // best stars when completed
	};

	static constexpr int Columns = 3;
	static constexpr int Rows = 3;

	static constexpr float CellSize = 40.0f;
	static constexpr float CellGap = 8.0f;

	void RebuildCells();
	void MoveSelection(int deltaColumn, int deltaRow);
	void LaunchSelected();

	int CellAt(float x, float y) const;            // -1 when outside the grid
	void CellTopLeft(int index, float& x, float& y) const;

	Context& context;

	std::array<Cell, Campaign::LevelCount> cells;
	int selected = 0;
	bool wasCloseRequested = false;
	std::function<void(int)> launchHandler;
};
