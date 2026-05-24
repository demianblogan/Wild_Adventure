#include "StateMachine.h"

void StateMachine::Push(std::unique_ptr<State> state)
{
	pendingActions.push_back({ ActionType::Push, std::move(state) });
}

void StateMachine::Pop()
{
	pendingActions.push_back({ ActionType::Pop, nullptr });
}

void StateMachine::Clear()
{
	pendingActions.push_back({ ActionType::Clear, nullptr });
}

void StateMachine::HandleEvent(const sf::Event& event)
{
	if (!stack.empty())
		stack.back()->HandleEvent(event);
}

void StateMachine::Update(float deltaTime)
{
	for (std::size_t i = stack.size(); i > 0; i--)
	{
		stack[i - 1]->Update(deltaTime);

		if (!stack[i - 1]->UpdatesStateBelow())
			break;
	}

	ApplyPendingActions();
}

void StateMachine::Render(float interpolationFactor)
{
	if (stack.empty())
		return;

	// Phase 1: find the lowest state index that must be drawn
	// (walk down while states are see-through to what's below them)
	std::size_t bottomVisible = stack.size() - 1;
	while (bottomVisible > 0 && stack[bottomVisible]->RendersStateBelow())
		bottomVisible--;

	// Phase 2: draw from that lowest state upward, so higher states layer on top
	for (std::size_t i = bottomVisible; i < stack.size(); i++)
		stack[i]->Render(interpolationFactor);
}

bool StateMachine::IsEmpty() const
{
	return stack.empty();
}

void StateMachine::ApplyPendingActions()
{
	for (auto& action : pendingActions)
	{
		switch (action.type)
		{
		case ActionType::Push:
			stack.push_back(std::move(action.state));
			break;

		case ActionType::Pop:
			if (!stack.empty())
				stack.pop_back();
			break;

		case ActionType::Clear:
			stack.clear();
			break;
		}
	}

	pendingActions.clear();
}