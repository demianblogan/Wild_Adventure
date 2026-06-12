#include "SelectLevelController.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "states/GameState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>

#include <cmath>
#include <memory>
#include <string>

namespace
{
	constexpr float W = static_cast<float>(VirtualScreen::WIDTH);
	constexpr float H = static_cast<float>(VirtualScreen::HEIGHT);

	constexpr float TITLE_Y = 22.0f;
	constexpr float GRID_TOP = 66.0f; // centers the 3x3 grid between the title and the hint
	constexpr float HINT_Y = 252.0f;

	// The star glyph sits in a 48x48 canvas with transparent padding; trim to its
	// opaque bounds and scale by exactly 1/4 (44x40 -> 11x10) so nearest-neighbour
	// downscaling samples every 4th texel and keeps the thin top tip visible.
	const sf::IntRect STAR_RECT({ 2, 4 }, { 44, 40 });
	constexpr float STAR_SCALE = 0.25f;
	constexpr float STAR_SPACING = 12.0f;

	constexpr float LOCK_DISPLAY_HEIGHT = 12.0f;

	const sf::Color GOLD(244, 199, 110, 255);
	const sf::Color OUTLINE(58, 42, 77, 255);
	const sf::Color CELL_NORMAL(200, 200, 200, 255);     // slightly dimmed
	const sf::Color CELL_SELECTED(255, 255, 255, 255);   // gentle highlight
	const sf::Color CELL_LOCKED(110, 110, 110, 255);
	const sf::Color NUMBER_LOCKED(170, 170, 170, 255);
	const sf::Color STAR_EMPTY(80, 80, 80, 200);

	void DrawCenteredText(sf::RenderTarget& target, const sf::Font& font,
		const std::string& str, unsigned int charSize,
		sf::Color fill, float cx, float cy)
	{
		sf::Text text(font, str, charSize);
		text.setFillColor(fill);
		text.setOutlineColor(OUTLINE);
		text.setOutlineThickness(1.0f);

		const sf::FloatRect bounds = text.getLocalBounds();
		text.setOrigin({
			bounds.position.x + bounds.size.x / 2.0f,
			bounds.position.y + bounds.size.y / 2.0f });
		text.setPosition({ std::floor(cx), std::floor(cy) });

		target.draw(text);
	}
}

SelectLevelController::SelectLevelController(Context& context)
	: context(context)
{}

void SelectLevelController::Open()
{
	wantsClose = false;
	RebuildCells();

	// Start on the next level to play (like Continue); fall back to the
	// furthest completed one, then to the first level.
	const int highest = context.campaign.GetHighestCompletedLevel();

	selected = 0;
	if (highest >= 1 && highest < Campaign::LEVEL_COUNT && cells[highest].selectable)
		selected = highest; // index of level highest+1
	else if (highest >= 1)
		selected = highest - 1;
}

void SelectLevelController::RebuildCells()
{
	for (int i = 0; i < Campaign::LEVEL_COUNT; i++)
	{
		const int number = i + 1;

		Cell& cell = cells[i];
		cell.completed = context.campaign.IsLevelCompleted(number);
		cell.stars = cell.completed ? context.campaign.GetStars(number) : 0;

		const bool unlocked = (number == 1) || context.campaign.IsLevelCompleted(number - 1);
		cell.selectable = unlocked && Campaign::LevelExists(number);
	}
}

void SelectLevelController::MoveSelection(int deltaColumn, int deltaRow)
{
	int column = selected % COLUMNS;
	int row = selected / COLUMNS;

	// Step in the chosen direction, skipping locked cells, until the edge.
	while (true)
	{
		column += deltaColumn;
		row += deltaRow;

		if (column < 0 || column >= COLUMNS || row < 0 || row >= ROWS)
			return;

		const int index = row * COLUMNS + column;

		if (cells[index].selectable)
		{
			if (index != selected)
			{
				selected = index;
				context.audioMixer.PlaySound("ui_hover");
			}
			return;
		}
	}
}

void SelectLevelController::LaunchSelected()
{
	if (!cells[selected].selectable)
		return;

	const int number = selected + 1;

	context.audioMixer.PlaySound("ui_press");
	wantsClose = true;

	if (launchHandler)
		launchHandler(number);
	else
		context.stateMachine.Push(std::make_unique<GameState>(context, Campaign::LevelPath(number), number));
}

void SelectLevelController::CellTopLeft(int index, float& x, float& y) const
{
	const float gridWidth = COLUMNS * CELL_SIZE + (COLUMNS - 1) * CELL_GAP;

	x = (W - gridWidth) / 2.0f + static_cast<float>(index % COLUMNS) * (CELL_SIZE + CELL_GAP);
	y = GRID_TOP + static_cast<float>(index / COLUMNS) * (CELL_SIZE + CELL_GAP);
}

int SelectLevelController::CellAt(float x, float y) const
{
	for (int i = 0; i < Campaign::LEVEL_COUNT; i++)
	{
		float cellX = 0.0f;
		float cellY = 0.0f;
		CellTopLeft(i, cellX, cellY);

		if (x >= cellX && x < cellX + CELL_SIZE && y >= cellY && y < cellY + CELL_SIZE)
			return i;
	}

	return -1;
}

void SelectLevelController::HandleEvent(const sf::Event& event)
{
	if (event.is<sf::Event::MouseMoved>())
	{
		const sf::Vector2f mouse = context.virtualScreen.GetMousePosition();
		const int index = CellAt(mouse.x, mouse.y);

		if (index >= 0 && cells[index].selectable && index != selected)
		{
			selected = index;
			context.audioMixer.PlaySound("ui_hover");
		}
	}
	else if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
	{
		if (pressed->button == sf::Mouse::Button::Left)
		{
			const sf::Vector2f mouse = context.virtualScreen.GetMousePosition();
			const int index = CellAt(mouse.x, mouse.y);

			if (index >= 0 && cells[index].selectable)
			{
				selected = index;
				LaunchSelected();
			}
		}
	}
}

void SelectLevelController::Update(float)
{
	Input& input = context.input;

	if (input.WasPressed(Action::MenuBack))
	{
		wantsClose = true;
		return;
	}

	if (input.WasPressed(Action::MenuLeft))
		MoveSelection(-1, 0);
	else if (input.WasPressed(Action::MenuRight))
		MoveSelection(1, 0);
	else if (input.WasPressed(Action::MenuUp))
		MoveSelection(0, -1);
	else if (input.WasPressed(Action::MenuDown))
		MoveSelection(0, 1);

	if (input.WasPressed(Action::MenuConfirm))
		LaunchSelected();
}

void SelectLevelController::Render(sf::RenderTarget& target)
{
	context.virtualScreen.SetCameraCenter(W / 2.0f, H / 2.0f);

	// Dim the moving backdrop so the grid reads well.
	sf::RectangleShape overlay({ W, H });
	overlay.setFillColor(sf::Color(0, 0, 0, 120));
	target.draw(overlay);

	const sf::Font& font = context.resources.fonts.Get("main");

	DrawCenteredText(target, font, "Select Level", 16, GOLD, W / 2.0f, TITLE_Y);

	Resources& resources = context.resources;
	const sf::Texture& normalBox = resources.textures.Get("container_background");
	const sf::Texture& goldenBox = resources.textures.Get("container_background2");
	const sf::Texture& lockTexture = resources.textures.Get("lock");
	const sf::Texture& starTexture = resources.textures.Get("mini_star");

	for (int i = 0; i < Campaign::LEVEL_COUNT; i++)
	{
		const Cell& cell = cells[i];

		float x = 0.0f;
		float y = 0.0f;
		CellTopLeft(i, x, y);

		const float centerX = x + CELL_SIZE / 2.0f;

		// Box: golden for a 3-star level, gray tint when locked, gentle
		// highlight on the selected cell.
		const sf::Texture& boxTexture = (cell.completed && cell.stars >= 3) ? goldenBox : normalBox;

		sf::Sprite box(boxTexture);
		box.setScale({ CELL_SIZE / static_cast<float>(boxTexture.getSize().x),
			CELL_SIZE / static_cast<float>(boxTexture.getSize().y) });
		box.setPosition({ x, y });

		if (!cell.selectable)
			box.setColor(CELL_LOCKED);
		else
			box.setColor(i == selected ? CELL_SELECTED : CELL_NORMAL);

		target.draw(box);

		// Level number: centered, or raised a little to make room for stars.
		const float numberY = cell.completed ? y + CELL_SIZE / 2.0f - 7.0f : y + CELL_SIZE / 2.0f;
		const sf::Color numberColor = cell.selectable ? sf::Color::White : NUMBER_LOCKED;
		DrawCenteredText(target, font, std::to_string(i + 1), 16, numberColor, centerX, numberY);

		// Earned and empty stars under the number on completed levels.
		if (cell.completed)
		{
			const float starWidth = static_cast<float>(STAR_RECT.size.x) * STAR_SCALE;
			const float starY = y + CELL_SIZE / 2.0f + 6.0f;

			for (int star = 0; star < 3; star++)
			{
				sf::Sprite sprite(starTexture);
				sprite.setTextureRect(STAR_RECT);
				sprite.setColor(star < cell.stars ? sf::Color::White : STAR_EMPTY);
				sprite.setScale({ STAR_SCALE, STAR_SCALE });
				sprite.setPosition({
					std::floor(centerX + (static_cast<float>(star) - 1.0f) * STAR_SPACING - starWidth / 2.0f),
					std::floor(starY) });
				target.draw(sprite);
			}
		}

		// Lock icon in the bottom-right corner of locked cells.
		if (!cell.selectable)
		{
			const float lockScale = LOCK_DISPLAY_HEIGHT / static_cast<float>(lockTexture.getSize().y);
			const float lockWidth = static_cast<float>(lockTexture.getSize().x) * lockScale;

			sf::Sprite lock(lockTexture);
			lock.setScale({ lockScale, lockScale });
			lock.setPosition({
				std::floor(x + CELL_SIZE - lockWidth - 3.0f),
				std::floor(y + CELL_SIZE - LOCK_DISPLAY_HEIGHT - 3.0f) });
			target.draw(lock);
		}
	}

	DrawCenteredText(target, font, "Back: Esc", 16, sf::Color(180, 180, 180, 255), W / 2.0f, HINT_Y);
}
