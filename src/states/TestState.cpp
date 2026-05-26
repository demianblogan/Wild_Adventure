#include "TestState.h"

#include "Context.h"
#include "core/Resources.h"
#include "components/Animation.h"
#include "components/AnimationSet.h"
#include "components/AnimationState.h"
#include "components/Sprite.h"
#include "components/Transform.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

TestState::TestState(Context& context)
	: State(context)
{
	// загрузить оба атласа врага (твои пути)
	context.resources.textures.Load("chicken_idle", "assets/textures/enemies/chicken/Idle (32x34).png");
	context.resources.textures.Load("chicken_run", "assets/textures/enemies/chicken/Run (32x34).png");

	enemy = registry.CreateEntity();
	registry.Add<Transform>(enemy, { 300.0f, 250.0f });
	registry.Add<Sprite>(enemy, { "chicken_idle" });

	AnimationSet set;
	set.animations["idle"] = { "chicken_idle", 13, 0.15f, true };
	set.animations["run"] = { "chicken_run",  14,  0.1f,  true };
	registry.Add<AnimationSet>(enemy, set);

	registry.Add<AnimationState>(enemy, { "idle" });
	registry.Add<Animation>(enemy, {});

	animationSystem.emplace(registry);
	renderSystem.emplace(registry, context.resources, context.window);
}

void TestState::HandleEvent(const sf::Event& event)
{}

void TestState::Update(float deltaTime)
{
	stateTimer += deltaTime;

	if (stateTimer >= 2.0f)
	{
		stateTimer = 0.0f;
		isRunning = !isRunning;
		registry.Get<AnimationState>(enemy).current = isRunning ? "run" : "idle";
	}

	animationSystem->Update(deltaTime);
}

void TestState::Render(float interpolationFactor)
{
	context.window.clear(sf::Color(30, 30, 46));
	renderSystem->Render(interpolationFactor);
}