#pragma once

#include "core/Campaign.h"

#include <array>
#include <functional>

class Context;

namespace sf
{
	class Event;
	class RenderTarget;
}

// Self-contained "Select Level" grid shown over the live main menu. The owner
// calls Open(), then forwards events / update / render while the grid is shown;
// when the user backs out (or launches a level) WantsClose() turns true.
class SelectLevelController
{
public:
	SelectLevelController(Context& context);

	void Open();
	bool WantsClose() const { return wantsClose; }

	// Called with the level number instead of launching it directly; the
	// owner routes the launch through the character select screen.
	void SetLaunchHandler(std::function<void(int)> handler) { launchHandler = std::move(handler); }

	void HandleEvent(const sf::Event& event);
	void Update(float deltaTime);
	void Render(sf::RenderTarget& target);

private:
	struct Cell
	{
		bool selectable = false; // unlocked and its .tmj file exists
		bool completed = false;
		int stars = 0;           // best stars when completed
	};

	static constexpr int COLUMNS = 3;
	static constexpr int ROWS = 3;

	static constexpr float CELL_SIZE = 40.0f;
	static constexpr float CELL_GAP = 8.0f;

	void RebuildCells();
	void MoveSelection(int deltaColumn, int deltaRow);
	void LaunchSelected();

	int CellAt(float x, float y) const;            // -1 when outside the grid
	void CellTopLeft(int index, float& x, float& y) const;

	Context& context;

	std::array<Cell, Campaign::LEVEL_COUNT> cells;
	int selected = 0;
	bool wantsClose = false;
	std::function<void(int)> launchHandler;
};
