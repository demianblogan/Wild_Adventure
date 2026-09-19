#pragma once

#include <SFML/Window/Event.hpp>

struct Context;

class State
{
public:
	State(Context& context, bool isRenderingStateBelow = false, bool isUpdatingStateBelow = false)
		: context(context)
		, isRenderingStateBelow(isRenderingStateBelow)
		, isUpdatingStateBelow(isUpdatingStateBelow)
	{}

	virtual ~State() = default;

	virtual void HandleEvent(const sf::Event& event) = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render(float interpolationFactor) = 0;

	bool IsRenderingStateBelow() const
	{
		return isRenderingStateBelow;
	}

	bool IsUpdatingStateBelow() const
	{
		return isUpdatingStateBelow;
	}

protected:
	Context& context;

private:
	bool isRenderingStateBelow;
	bool isUpdatingStateBelow;
};