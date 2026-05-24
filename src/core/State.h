#pragma once

#include <SFML/Window/Event.hpp>

class Context;

class State
{
public:
	State(Context& context, bool rendersStateBelow = false, bool updatesStateBelow = false)
		: context(context)
		, rendersStateBelow(rendersStateBelow)
		, updatesStateBelow(updatesStateBelow)
	{}

	virtual ~State() = default;

	virtual void HandleEvent(const sf::Event& event) = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render(float interpolationFactor) = 0;

	bool RendersStateBelow() const
	{
		return rendersStateBelow;
	}

	bool UpdatesStateBelow() const
	{
		return updatesStateBelow;
	}

protected:
	Context& context;

private:
	bool rendersStateBelow;
	bool updatesStateBelow;
};