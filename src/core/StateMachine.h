#pragma once

#include "State.h"

#include <functional>
#include <memory>
#include <vector>

namespace sf
{
	class Event;
}

class StateMachine
{
public:
	void Push(std::unique_ptr<State> state);
	void Pop();
	void Clear();

	void HandleEvent(const sf::Event& event);
	void Update(float deltaTime);
	void Render(float interpolationFactor);

	bool IsEmpty() const;

private:
	void ApplyPendingActions();

	enum class ActionType
	{
		Push,
		Pop,
		Clear
	};

	struct PendingAction
	{
		ActionType type;
		std::unique_ptr<State> state;
	};

	std::vector<std::unique_ptr<State>> stack;
	std::vector<PendingAction> pendingActions;
};