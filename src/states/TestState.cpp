#include "TestState.h"

#include "Context.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

TestState::TestState(Context& context)
	: State(context)
{}

void TestState::HandleEvent(const sf::Event& event)
{}

void TestState::Update(float deltaTime)
{}

void TestState::Render(float interpolationFactor)
{
	context.window.clear(sf::Color(30, 30, 46));
}