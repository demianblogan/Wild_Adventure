#include "Transition.h"

#include "core/VirtualScreen.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <cmath>

void Transition::StartReveal()
{
	mode = Mode::Reveal;
	elapsedTime = 0.0f;
}

void Transition::StartCover()
{
	mode = Mode::Cover;
	elapsedTime = 0.0f;
}

Transition::Mode Transition::GetMode() const
{
	return mode;
}

void Transition::Update(float deltaTime)
{
	if (mode == Mode::Idle || mode == Mode::Done)
		return;

	elapsedTime += deltaTime;

	const float total = (ColumnCount - 1) * ColumnDelay + ColumnDuration;

	if (elapsedTime >= total)
	{
		if (mode == Mode::Reveal)
			mode = Mode::Idle;
		else if (mode == Mode::Cover)
			mode = Mode::Done;
	}
}

float Transition::ColumnFillFactor(int column) const
{
	const float start = column * ColumnDelay;
	const float localProgress = std::clamp((elapsedTime - start) / ColumnDuration, 0.0f, 1.0f);

	// Reveal: diamonds shrink (1 -> 0). Cover/Done: diamonds grow (0 -> 1).
	if (mode == Mode::Cover || mode == Mode::Done)
		return localProgress;

	return 1.0f - localProgress;
}

void Transition::Draw(sf::RenderTarget& target)
{
	if (mode == Mode::Idle)
		return;

	const float cellSize = static_cast<float>(VirtualScreen::Width) / ColumnCount;
	const int rowCount = static_cast<int>(std::ceil(static_cast<float>(VirtualScreen::Height) / cellSize));
	const float maxRadius = cellSize; // half-diagonal == spacing -> diamonds meet, full coverage

	sf::ConvexShape diamond;
	diamond.setPointCount(4);
	diamond.setFillColor(sf::Color::Black);

	for (int column = 0; column < ColumnCount; ++column)
	{
		const float factor = ColumnFillFactor(column);

		if (factor <= 0.0f)
			continue;

		const float radius = maxRadius * factor;
		const float centerX = (column + 0.5f) * cellSize;

		for (int row = 0; row < rowCount; ++row)
		{
			const float centerY = (row + 0.5f) * cellSize;

			diamond.setPoint(0, { centerX, centerY - radius });
			diamond.setPoint(1, { centerX + radius, centerY });
			diamond.setPoint(2, { centerX, centerY + radius });
			diamond.setPoint(3, { centerX - radius, centerY });

			target.draw(diamond);
		}
	}
}