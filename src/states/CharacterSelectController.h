#pragma once

#include <SFML/Graphics/Rect.hpp>

class Context;

namespace sf
{
	class Event;
	class RenderTarget;
}

// "Select Character" screen shown over the live main menu before a level
// starts: New Game, Continue and Select Level all route through it. The
// arrows cycle through the skins; locked ones are drawn gray with a lock
// badge, and Play stays disabled until an unlocked skin is selected.
// When the user backs out or presses Play, WantsClose() turns true.
class CharacterSelectController
{
public:
	CharacterSelectController(Context& context);

	void Open(int levelNumber); // the level Play will launch
	bool WantsClose() const { return wantsClose; }

	void HandleEvent(const sf::Event& event);
	void Update(float deltaTime);
	void Render(sf::RenderTarget& target);

private:
	enum class Focus { Carousel, PlayButton, BackButton };

	bool IsUnlocked(int skinIndex) const;
	void MoveSkin(int delta);
	void SetFocus(Focus newFocus);
	void Activate();
	void Launch();

	// Screen-space hit boxes; the layout is fixed, so they are constants.
	sf::FloatRect LeftArrowRect() const;
	sf::FloatRect RightArrowRect() const;
	sf::FloatRect PlayRect() const;
	sf::FloatRect BackRect() const;

	Context& context;

	int levelNumber = 1;
	int selectedSkin = 0;
	Focus focus = Focus::Carousel;
	bool wantsClose = false;
};
