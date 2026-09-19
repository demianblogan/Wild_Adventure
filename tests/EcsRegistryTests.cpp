#include "doctest/doctest.h"

#include "core/ecs/Registry.h"

namespace
{
	struct Position { float x = 0.0f; float y = 0.0f; };
	struct Velocity { float dx = 0.0f; float dy = 0.0f; };
}

TEST_SUITE("ECS::Registry")
{
	TEST_CASE("CreateEntity returns distinct, increasing ids")
	{
		ECS::Registry registry;
		const ECS::Entity a = registry.CreateEntity();
		const ECS::Entity b = registry.CreateEntity();

		CHECK(a != b);
		CHECK(b > a);
	}

	TEST_CASE("Add, Has and Get round-trip a component")
	{
		ECS::Registry registry;
		const ECS::Entity entity = registry.CreateEntity();

		CHECK_FALSE(registry.Has<Position>(entity));

		registry.Add<Position>(entity, { 3.0f, 4.0f });

		REQUIRE(registry.Has<Position>(entity));
		CHECK(registry.Get<Position>(entity).x == doctest::Approx(3.0f));
		CHECK(registry.Get<Position>(entity).y == doctest::Approx(4.0f));
	}

	TEST_CASE("RemoveFrom keeps the other entities' components intact (swap-remove correctness)")
	{
		ECS::Registry registry;
		const ECS::Entity first = registry.CreateEntity();
		const ECS::Entity second = registry.CreateEntity();
		const ECS::Entity third = registry.CreateEntity();

		registry.Add<Position>(first, { 1.0f, 1.0f });
		registry.Add<Position>(second, { 2.0f, 2.0f });
		registry.Add<Position>(third, { 3.0f, 3.0f });

		// Removing the middle entry forces the pool to swap its last element
		// into the freed slot; every surviving entity must still resolve to
		// its own data afterwards, not a neighbour's.
		registry.RemoveFrom<Position>(second);

		CHECK_FALSE(registry.Has<Position>(second));
		REQUIRE(registry.Has<Position>(first));
		REQUIRE(registry.Has<Position>(third));
		CHECK(registry.Get<Position>(first).x == doctest::Approx(1.0f));
		CHECK(registry.Get<Position>(third).x == doctest::Approx(3.0f));
	}

	TEST_CASE("DestroyEntity removes the entity from every component pool it was in")
	{
		ECS::Registry registry;
		const ECS::Entity entity = registry.CreateEntity();

		registry.Add<Position>(entity, {});
		registry.Add<Velocity>(entity, {});

		registry.DestroyEntity(entity);

		CHECK_FALSE(registry.Has<Position>(entity));
		CHECK_FALSE(registry.Has<Velocity>(entity));
	}

	TEST_CASE("ForEach visits only entities that have every required component")
	{
		ECS::Registry registry;
		const ECS::Entity both = registry.CreateEntity();
		const ECS::Entity positionOnly = registry.CreateEntity();

		registry.Add<Position>(both, { 5.0f, 0.0f });
		registry.Add<Velocity>(both, { 1.0f, 0.0f });
		registry.Add<Position>(positionOnly, { 9.0f, 0.0f });

		int visitCount = 0;
		registry.ForEach<Position, Velocity>(
			[&](ECS::Entity entity, Position& position, Velocity&)
			{
				visitCount++;
				CHECK(entity == both);
				CHECK(position.x == doctest::Approx(5.0f));
			});

		CHECK(visitCount == 1);
	}
}
