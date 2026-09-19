#include "SelectLevelController.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "localization/LocalizationManager.h"
#include "states/GameState.h"
#include "ui/Image.h"
#include "ui/Label.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Window/Event.hpp>

#include <cmath>
#include <memory>
#include <string>

namespace
{
	constexpr float ScreenWidth = static_cast<float>(VirtualScreen::Width);
	constexpr float ScreenHeight = static_cast<float>(VirtualScreen::Height);

	constexpr float GridTop = 66.0f; // centers the 3x3 grid between the title and the Back button

	// Back button under the grid, styled like the one on Select Character.
	constexpr float ButtonWidth = 90.0f;
	constexpr float ButtonHeight = 26.0f;
	constexpr float ButtonY = 234.0f;

	// The star glyph sits in a 48x48 canvas with transparent padding; trim to its
	// opaque bounds and scale by exactly 1/4 (44x40 -> 11x10) so nearest-neighbour
	// downscaling samples every 4th texel and keeps the thin top tip visible.
	const sf::IntRect StarRect({ 2, 4 }, { 44, 40 });
	constexpr float StarScale = 0.25f;
	constexpr float StarSpacing = 12.0f;

	constexpr float LockDisplayHeight = 12.0f;

	const sf::Color Outline(58, 42, 77, 255);
	const sf::Color CellNormal(200, 200, 200, 255);     // slightly dimmed
	const sf::Color CellSelected(255, 255, 255, 255);   // gentle highlight
	const sf::Color CellLocked(110, 110, 110, 255);
	const sf::Color NumberLocked(170, 170, 170, 255);
	const sf::Color StarEmpty(80, 80, 80, 200);

	void DrawCenteredText(sf::RenderTarget& target, const sf::Font& font,
		const std::string& str, unsigned int charSize,
		sf::Color fill, float cx, float cy)
	{
		sf::Text text(font, sf::String::fromUtf8(str.begin(), str.end()), charSize);
		text.setFillColor(fill);
		text.setOutlineColor(Outline);
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
	, chromeLoader(context.resources)
{
	chromeLoader.SetLocalization(context.localization);
	chrome = chromeLoader.LoadFromFile("data/ui/menu/select_level.json");
	lastLocalizationRevision = context.localization.Revision();
}

void SelectLevelController::Open()
{
	wasCloseRequested = false;
	focus = Focus::Grid;
	RebuildCells();

	// The language can only change while this screen is closed (it and
	// Settings are mutually exclusive views inside MenuState), so it is
	// enough to check for a stale chrome here rather than every frame.
	if (context.localization.Revision() != lastLocalizationRevision)
	{
		lastLocalizationRevision = context.localization.Revision();
		chrome = chromeLoader.LoadFromFile("data/ui/menu/select_level.json");
	}

	// Start on the next level to play (like Continue); fall back to the
	// furthest completed one, then to the first level.
	const int highest = context.campaign.GetHighestCompletedLevel();

	selected = 0;
	if (highest >= 1 && highest < Campaign::LevelCount && cells[highest].isSelectable)
		selected = highest; // index of level highest+1
	else if (highest >= 1)
		selected = highest - 1;
}

void SelectLevelController::RebuildCells()
{
	for (int i = 0; i < Campaign::LevelCount; i++)
	{
		const int number = i + 1;

		Cell& cell = cells[i];
		cell.isCompleted = context.campaign.IsLevelCompleted(number);
		cell.stars = cell.isCompleted ? context.campaign.GetStars(number) : 0;

		const bool unlocked = (number == 1) || context.campaign.IsLevelCompleted(number - 1);
		cell.isSelectable = unlocked && Campaign::LevelExists(number);
	}
}

void SelectLevelController::MoveSelection(int deltaColumn, int deltaRow)
{
	int column = selected % Columns;
	int row = selected / Columns;

	// Step in the chosen direction, skipping locked cells, until the edge.
	while (true)
	{
		column += deltaColumn;
		row += deltaRow;

		if (column < 0 || column >= Columns || row < 0 || row >= Rows)
			return;

		const int index = row * Columns + column;

		if (index >= Campaign::LevelCount)
			return;

		if (cells[index].isSelectable)
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

void SelectLevelController::SetFocus(Focus newFocus)
{
	if (focus == newFocus)
		return;

	focus = newFocus;
	context.audioMixer.PlaySound("ui_hover");
}

void SelectLevelController::LaunchSelected()
{
	if (!cells[selected].isSelectable)
		return;

	const int number = selected + 1;

	context.audioMixer.PlaySound("ui_press");
	wasCloseRequested = true;

	if (launchHandler)
		launchHandler(number);
	else
		context.stateMachine.Push(std::make_unique<GameState>(context, Campaign::LevelPath(number), number));
}

void SelectLevelController::CellTopLeft(int index, float& x, float& y) const
{
	const float gridWidth = Columns * CellSize + (Columns - 1) * CellGap;

	x = (ScreenWidth - gridWidth) / 2.0f + static_cast<float>(index % Columns) * (CellSize + CellGap);
	y = GridTop + static_cast<float>(index / Columns) * (CellSize + CellGap);
}

int SelectLevelController::CellAt(float x, float y) const
{
	for (int i = 0; i < Campaign::LevelCount; i++)
	{
		float cellX = 0.0f;
		float cellY = 0.0f;
		CellTopLeft(i, cellX, cellY);

		if (x >= cellX && x < cellX + CellSize && y >= cellY && y < cellY + CellSize)
			return i;
	}

	return -1;
}

sf::FloatRect SelectLevelController::BackRect() const
{
	return { { (ScreenWidth - ButtonWidth) / 2.0f, ButtonY }, { ButtonWidth, ButtonHeight } };
}

void SelectLevelController::HandleEvent(const sf::Event& event)
{
	if (event.is<sf::Event::MouseMoved>())
	{
		const sf::Vector2f mouse = context.virtualScreen.GetMousePosition();

		if (BackRect().contains(mouse))
		{
			SetFocus(Focus::BackButton);
			return;
		}

		const int index = CellAt(mouse.x, mouse.y);

		if (index >= 0 && cells[index].isSelectable)
		{
			SetFocus(Focus::Grid);

			if (index != selected)
			{
				selected = index;
				context.audioMixer.PlaySound("ui_hover");
			}
		}
	}
	else if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
	{
		if (pressed->button == sf::Mouse::Button::Left)
		{
			const sf::Vector2f mouse = context.virtualScreen.GetMousePosition();

			if (BackRect().contains(mouse))
			{
				context.audioMixer.PlaySound("ui_press");
				wasCloseRequested = true;
				return;
			}

			const int index = CellAt(mouse.x, mouse.y);

			if (index >= 0 && cells[index].isSelectable)
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
		wasCloseRequested = true;
		return;
	}

	if (focus == Focus::Grid)
	{
		if (input.WasPressed(Action::MenuLeft))
			MoveSelection(-1, 0);
		else if (input.WasPressed(Action::MenuRight))
			MoveSelection(1, 0);
		else if (input.WasPressed(Action::MenuUp))
			MoveSelection(0, -1);
		else if (input.WasPressed(Action::MenuDown))
		{
			// The bottom row has nowhere further down to go: leave the grid
			// for the Back button instead of doing nothing.
			if (selected / Columns == Rows - 1)
				SetFocus(Focus::BackButton);
			else
				MoveSelection(0, 1);
		}

		if (input.WasPressed(Action::MenuConfirm))
			LaunchSelected();
	}
	else // Focus::BackButton
	{
		if (input.WasPressed(Action::MenuUp))
			SetFocus(Focus::Grid);

		if (input.WasPressed(Action::MenuConfirm))
		{
			context.audioMixer.PlaySound("ui_press");
			wasCloseRequested = true;
		}
	}
}

void SelectLevelController::Render(sf::RenderTarget& target)
{
	context.virtualScreen.SetCameraCenter(ScreenWidth / 2.0f, ScreenHeight / 2.0f);

	// Dim the moving backdrop so the grid reads well.
	sf::RectangleShape overlay({ ScreenWidth, ScreenHeight });
	overlay.setFillColor(sf::Color(0, 0, 0, 120));
	target.draw(overlay);

	const sf::Font& font = context.resources.fonts.Get("main");

	if (auto* backBackground = dynamic_cast<UI::Image*>(chrome->FindByName("back_background")))
		backBackground->SetColor(focus == Focus::BackButton ? CellSelected : CellNormal);
	if (auto* backLabel = dynamic_cast<UI::Label*>(chrome->FindByName("back_label")))
		backLabel->SetColor(sf::Color::White);

	chrome->Draw(target, { 0.0f, 0.0f }, { ScreenWidth, ScreenHeight });

	Resources& resources = context.resources;
	const sf::Texture& normalBox = resources.textures.Get("container_background");
	const sf::Texture& goldenBox = resources.textures.Get("container_background2");
	const sf::Texture& lockTexture = resources.textures.Get("lock");
	const sf::Texture& starTexture = resources.textures.Get("mini_star");

	for (int i = 0; i < Campaign::LevelCount; i++)
	{
		const Cell& cell = cells[i];

		float x = 0.0f;
		float y = 0.0f;
		CellTopLeft(i, x, y);

		const float centerX = x + CellSize / 2.0f;

		// Box: golden for a 3-star level, gray tint when locked, gentle
		// highlight on the selected cell.
		const sf::Texture& boxTexture = (cell.isCompleted && cell.stars >= 3) ? goldenBox : normalBox;

		sf::Sprite box(boxTexture);
		box.setScale({ CellSize / static_cast<float>(boxTexture.getSize().x),
			CellSize / static_cast<float>(boxTexture.getSize().y) });
		box.setPosition({ x, y });

		if (!cell.isSelectable)
			box.setColor(CellLocked);
		else
			box.setColor(i == selected ? CellSelected : CellNormal);

		target.draw(box);

		// Level number: centered, or raised a little to make room for stars.
		const float numberY = cell.isCompleted ? y + CellSize / 2.0f - 7.0f : y + CellSize / 2.0f;
		const sf::Color numberColor = cell.isSelectable ? sf::Color::White : NumberLocked;
		DrawCenteredText(target, font, std::to_string(i + 1), 16, numberColor, centerX, numberY);

		// Earned and empty stars under the number on completed levels.
		if (cell.isCompleted)
		{
			const float starWidth = static_cast<float>(StarRect.size.x) * StarScale;
			const float starY = y + CellSize / 2.0f + 6.0f;

			for (int star = 0; star < 3; star++)
			{
				sf::Sprite sprite(starTexture);
				sprite.setTextureRect(StarRect);
				sprite.setColor(star < cell.stars ? sf::Color::White : StarEmpty);
				sprite.setScale({ StarScale, StarScale });
				sprite.setPosition({
					std::floor(centerX + (static_cast<float>(star) - 1.0f) * StarSpacing - starWidth / 2.0f),
					std::floor(starY) });
				target.draw(sprite);
			}
		}

		// Lock icon in the bottom-right corner of locked cells.
		if (!cell.isSelectable)
		{
			const float lockScale = LockDisplayHeight / static_cast<float>(lockTexture.getSize().y);
			const float lockWidth = static_cast<float>(lockTexture.getSize().x) * lockScale;

			sf::Sprite lock(lockTexture);
			lock.setScale({ lockScale, lockScale });
			lock.setPosition({
				std::floor(x + CellSize - lockWidth - 3.0f),
				std::floor(y + CellSize - LockDisplayHeight - 3.0f) });
			target.draw(lock);
		}
	}

}
