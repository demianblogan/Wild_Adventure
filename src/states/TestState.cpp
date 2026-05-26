#include "TestState.h"

#include "Context.h"
#include "core/Resources.h"
#include "components/Animation.h"
#include "components/PreviousTransform.h"
#include "components/Sprite.h"
#include "components/Transform.h"
#include "components/Velocity.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

TestState::TestState(Context& context)
	: State(context)
{
	context.resources.textures.Load("run", "assets/textures/main_characters/mask_dude/Run (32x32).png");  // <-- твой путь

	Entity hero = registry.CreateEntity();
	registry.Add<Transform>(hero, { 100.0f, 300.0f });
	registry.Add<PreviousTransform>(hero, { 100.0f, 300.0f });
	registry.Add<Velocity>(hero, { 80.0f, 0.0f });
	registry.Add<Sprite>(hero, { "run" });
	registry.Add<Animation>(hero, { 12, 0.05f, true });

	movementSystem.emplace(registry);
	animationSystem.emplace(registry);
	renderSystem.emplace(registry, context.resources, context.window);
}

void TestState::HandleEvent(const sf::Event& event)
{}

void TestState::Update(float deltaTime)
{
	movementSystem->Update(deltaTime);
	animationSystem->Update(deltaTime);
}

void TestState::Render(float interpolationFactor)
{
	context.window.clear(sf::Color(30, 30, 46));
	renderSystem->Render(interpolationFactor);
}