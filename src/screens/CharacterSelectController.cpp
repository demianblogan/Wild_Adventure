#include "CharacterSelectController.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Campaign.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/Skins.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "graphics/NineSlice.h"
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
	constexpr float ScreenWidth = static_cast<float>(VirtualScreen::Width);
	constexpr float ScreenHeight = static_cast<float>(VirtualScreen::Height);

	// Header label, styled like the settings panel headers ("Audio", ...).
	constexpr float HeaderY = 38.0f;
	constexpr float HeaderWidth = 200.0f;
	constexpr float HeaderHeight = 30.0f;

	// Carousel box with the arrows, the portrait, the skin name and, on
	// locked skins, the unlock requirement line.
	constexpr float BoxWidth = 280.0f;
	constexpr float BoxHeight = 132.0f;
	constexpr float BoxTop = 64.0f;

	constexpr float PortraitScale = 2.0f;  // 32x32 idle frame -> 64x64
	constexpr float PortraitCenterY = BoxTop + 44.0f;
	constexpr float NameY = BoxTop + 90.0f;

	constexpr float ArrowWidth = 14.0f;    // 7x12 texture at x2
	constexpr float ArrowHeight = 24.0f;
	constexpr float ArrowInset = 16.0f;    // from the box's inner edge
	constexpr float ArrowHitPadding = 6.0f;

	constexpr float LockDisplayHeight = 12.0f;

	// Play / Back buttons under the box.
	constexpr float ButtonWidth = 90.0f;
	constexpr float ButtonHeight = 26.0f;
	constexpr float ButtonY = 206.0f;
	constexpr float ButtonSpacing = 110.0f; // center to center

	constexpr float HintY = 252.0f;

	// 9-slice borders in texture pixels, matching the menu UI configs
	// (label_background uses [32, 2], container_background uses 15).
	constexpr float LabelBorderX = 32.0f;
	constexpr float LabelBorderY = 2.0f;
	constexpr float ContainerBorder = 15.0f;

	const sf::Color Gold(244, 199, 110, 255);
	const sf::Color Outline(58, 42, 77, 255);
	const sf::Color BoxNormal(200, 200, 200, 255);
	const sf::Color BoxSelected(255, 255, 255, 255);
	const sf::Color BoxDisabled(110, 110, 110, 255);
	const sf::Color TextDisabled(170, 170, 170, 255);
	const sf::Color ArrowNormal(200, 200, 200, 255);
	const sf::Color ArrowHighlighted(255, 255, 255, 255);
	const sf::Color PortraitLocked(70, 70, 70, 255);

	void DrawCenteredText(sf::RenderTarget& target, const sf::Font& font,
		const std::string& str, unsigned int charSize,
		sf::Color fill, float cx, float cy)
	{
		sf::Text text(font, str, charSize);
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

CharacterSelectController::CharacterSelectController(Context& context)
	: context(context)
{}

void CharacterSelectController::Open(int level)
{
	levelNumber = level;
	wantsClose = false;
	focus = Focus::Carousel;

	// Start on the skin the player used last; fall back to the default when
	// it is unknown or no longer unlocked (e.g. after deleting the saves).
	selectedSkin = 0;

	const auto& skins = AllSkins();
	for (int i = 0; i < static_cast<int>(skins.size()); i++)
	{
		if (skins[i].id == context.campaign.GetSelectedSkin() && IsUnlocked(i))
			selectedSkin = i;
	}
}

bool CharacterSelectController::IsUnlocked(int skinIndex) const
{
	return context.campaign.CountThreeStarLevels() >= AllSkins()[skinIndex].requiredThreeStars;
}

void CharacterSelectController::MoveSkin(int delta)
{
	const int count = static_cast<int>(AllSkins().size());
	selectedSkin = (selectedSkin + delta + count) % count;
	context.audioMixer.PlaySound("ui_hover");
}

void CharacterSelectController::SetFocus(Focus newFocus)
{
	if (focus == newFocus)
		return;

	focus = newFocus;
	context.audioMixer.PlaySound("ui_hover");
}

void CharacterSelectController::Activate()
{
	switch (focus)
	{
	case Focus::Carousel:
	case Focus::PlayButton:
		if (IsUnlocked(selectedSkin))
			Launch();
		break;

	case Focus::BackButton:
		context.audioMixer.PlaySound("ui_press");
		wantsClose = true;
		break;
	}
}

void CharacterSelectController::Launch()
{
	context.campaign.SetSelectedSkin(AllSkins()[selectedSkin].id);
	context.audioMixer.PlaySound("ui_press");
	wantsClose = true;

	context.stateMachine.Push(std::make_unique<GameState>(
		context, Campaign::LevelPath(levelNumber), levelNumber));
}

sf::FloatRect CharacterSelectController::LeftArrowRect() const
{
	const float boxLeft = (ScreenWidth - BoxWidth) / 2.0f;
	return { { boxLeft + ArrowInset - ArrowHitPadding, PortraitCenterY - ArrowHeight / 2.0f - ArrowHitPadding },
		{ ArrowWidth + ArrowHitPadding * 2.0f, ArrowHeight + ArrowHitPadding * 2.0f } };
}

sf::FloatRect CharacterSelectController::RightArrowRect() const
{
	const float boxRight = (ScreenWidth + BoxWidth) / 2.0f;
	return { { boxRight - ArrowInset - ArrowWidth - ArrowHitPadding, PortraitCenterY - ArrowHeight / 2.0f - ArrowHitPadding },
		{ ArrowWidth + ArrowHitPadding * 2.0f, ArrowHeight + ArrowHitPadding * 2.0f } };
}

sf::FloatRect CharacterSelectController::PlayRect() const
{
	return { { ScreenWidth / 2.0f - ButtonSpacing / 2.0f - ButtonWidth / 2.0f, ButtonY },
		{ ButtonWidth, ButtonHeight } };
}

sf::FloatRect CharacterSelectController::BackRect() const
{
	return { { ScreenWidth / 2.0f + ButtonSpacing / 2.0f - ButtonWidth / 2.0f, ButtonY },
		{ ButtonWidth, ButtonHeight } };
}

void CharacterSelectController::HandleEvent(const sf::Event& event)
{
	if (event.is<sf::Event::MouseMoved>())
	{
		const sf::Vector2f mouse = context.virtualScreen.GetMousePosition();

		if (LeftArrowRect().contains(mouse) || RightArrowRect().contains(mouse))
			SetFocus(Focus::Carousel);
		else if (PlayRect().contains(mouse))
			SetFocus(Focus::PlayButton);
		else if (BackRect().contains(mouse))
			SetFocus(Focus::BackButton);
	}
	else if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
	{
		if (pressed->button != sf::Mouse::Button::Left)
			return;

		const sf::Vector2f mouse = context.virtualScreen.GetMousePosition();

		if (LeftArrowRect().contains(mouse))
			MoveSkin(-1);
		else if (RightArrowRect().contains(mouse))
			MoveSkin(1);
		else if (PlayRect().contains(mouse))
		{
			focus = Focus::PlayButton;
			Activate();
		}
		else if (BackRect().contains(mouse))
		{
			focus = Focus::BackButton;
			Activate();
		}
	}
}

void CharacterSelectController::Update(float)
{
	Input& input = context.input;

	if (input.WasPressed(Action::MenuBack))
	{
		wantsClose = true;
		return;
	}

	if (input.WasPressed(Action::MenuLeft))
	{
		if (focus == Focus::Carousel)
			MoveSkin(-1);
		else
			SetFocus(Focus::PlayButton);
	}
	else if (input.WasPressed(Action::MenuRight))
	{
		if (focus == Focus::Carousel)
			MoveSkin(1);
		else
			SetFocus(Focus::BackButton);
	}
	else if (input.WasPressed(Action::MenuDown))
	{
		if (focus == Focus::Carousel)
			SetFocus(Focus::PlayButton);
	}
	else if (input.WasPressed(Action::MenuUp))
	{
		if (focus != Focus::Carousel)
			SetFocus(Focus::Carousel);
	}

	if (input.WasPressed(Action::MenuConfirm))
		Activate();
}

void CharacterSelectController::Render(sf::RenderTarget& target)
{
	context.virtualScreen.SetCameraCenter(ScreenWidth / 2.0f, ScreenHeight / 2.0f);

	// Dim the moving backdrop so the panel reads well.
	sf::RectangleShape overlay({ ScreenWidth, ScreenHeight });
	overlay.setFillColor(sf::Color(0, 0, 0, 120));
	target.draw(overlay);

	Resources& resources = context.resources;
	const sf::Font& font = resources.fonts.Get("main");

	const Skin& skin = AllSkins()[selectedSkin];
	const bool unlocked = IsUnlocked(selectedSkin);

	// Header, styled like the settings panel headers.
	DrawNineSlice(target, resources.textures.Get("label_background"),
		{ (ScreenWidth - HeaderWidth) / 2.0f, HeaderY - HeaderHeight / 2.0f }, { HeaderWidth, HeaderHeight },
		LabelBorderX, LabelBorderY, LabelBorderX, LabelBorderY);
	DrawCenteredText(target, font, "Select Character", 16, Gold, ScreenWidth / 2.0f, HeaderY);

	// Carousel box.
	DrawNineSlice(target, resources.textures.Get("container_background"),
		{ (ScreenWidth - BoxWidth) / 2.0f, BoxTop }, { BoxWidth, BoxHeight },
		ContainerBorder, ContainerBorder, ContainerBorder, ContainerBorder);

	// Arrows: highlighted while the carousel has the focus.
	const sf::Color arrowColor = (focus == Focus::Carousel) ? ArrowHighlighted : ArrowNormal;

	sf::Sprite leftArrow(resources.textures.Get("arrow_left_normal"));
	leftArrow.setScale({ 2.0f, 2.0f });
	leftArrow.setPosition({ LeftArrowRect().position.x + ArrowHitPadding, LeftArrowRect().position.y + ArrowHitPadding });
	leftArrow.setColor(arrowColor);
	target.draw(leftArrow);

	sf::Sprite rightArrow(resources.textures.Get("arrow_right_normal"));
	rightArrow.setScale({ 2.0f, 2.0f });
	rightArrow.setPosition({ RightArrowRect().position.x + ArrowHitPadding, RightArrowRect().position.y + ArrowHitPadding });
	rightArrow.setColor(arrowColor);
	target.draw(rightArrow);

	// Portrait: the first frame of the skin's Idle sheet; gray when locked.
	const sf::Texture& idleTexture = resources.textures.Get(skin.id + "_idle");

	sf::Sprite portrait(idleTexture);
	portrait.setTextureRect(sf::IntRect({ 0, 0 }, { 32, 32 }));
	portrait.setScale({ PortraitScale, PortraitScale });
	portrait.setOrigin({ 16.0f, 16.0f });
	portrait.setPosition({ ScreenWidth / 2.0f, PortraitCenterY });
	portrait.setColor(unlocked ? sf::Color::White : PortraitLocked);
	target.draw(portrait);

	// Lock badge in the bottom-right corner of the portrait, like the
	// locked cells on the Select Level grid.
	if (!unlocked)
	{
		const sf::Texture& lockTexture = resources.textures.Get("lock");
		const float lockScale = LockDisplayHeight / static_cast<float>(lockTexture.getSize().y);

		sf::Sprite lock(lockTexture);
		lock.setScale({ lockScale, lockScale });
		lock.setPosition({
			std::floor(ScreenWidth / 2.0f + 16.0f * PortraitScale - static_cast<float>(lockTexture.getSize().x) * lockScale),
			std::floor(PortraitCenterY + 16.0f * PortraitScale - LockDisplayHeight) });
		target.draw(lock);
	}

	// Skin name; locked skins also show the unlock requirement.
	DrawCenteredText(target, font, skin.displayName, 16,
		unlocked ? sf::Color::White : TextDisabled, ScreenWidth / 2.0f, NameY);

	if (!unlocked)
	{
		const std::string requirement =
			std::to_string(skin.requiredThreeStars) + " levels with 3 stars ("
			+ std::to_string(context.campaign.CountThreeStarLevels()) + "/"
			+ std::to_string(skin.requiredThreeStars) + ")";
		DrawCenteredText(target, font, requirement, 16, TextDisabled, ScreenWidth / 2.0f, NameY + 16.0f);
	}

	// Play and Back buttons. Play goes gray while a locked skin is shown.
	const bool playEnabled = unlocked;

	const sf::Color playBoxColor = !playEnabled ? BoxDisabled
		: (focus == Focus::PlayButton ? BoxSelected : BoxNormal);
	DrawNineSlice(target, resources.textures.Get("container_background"),
		PlayRect().position, PlayRect().size,
		ContainerBorder, ContainerBorder, ContainerBorder, ContainerBorder, playBoxColor);
	DrawCenteredText(target, font, "Play", 16,
		playEnabled ? sf::Color::White : TextDisabled,
		PlayRect().position.x + ButtonWidth / 2.0f, ButtonY + ButtonHeight / 2.0f);

	const sf::Color backBoxColor = (focus == Focus::BackButton) ? BoxSelected : BoxNormal;
	DrawNineSlice(target, resources.textures.Get("container_background"),
		BackRect().position, BackRect().size,
		ContainerBorder, ContainerBorder, ContainerBorder, ContainerBorder, backBoxColor);
	DrawCenteredText(target, font, "Back", 16, sf::Color::White,
		BackRect().position.x + ButtonWidth / 2.0f, ButtonY + ButtonHeight / 2.0f);

	DrawCenteredText(target, font, "Back: Esc", 16, sf::Color(180, 180, 180, 255), ScreenWidth / 2.0f, HintY);
}
