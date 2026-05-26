#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponentPool.h"

#include <cassert>
#include <utility>
#include <vector>

// Classic sparse-set under readable names:
// components       - packed component data, no gaps
// componentIndexes - entity ID -> position of its component in `components`
// ownerEntities    - position in `components` -> entity that owns it
template <typename T>
class ComponentPool : public IComponentPool
{
public:
	void Add(Entity entity, T component)
	{
		assert(!Has(entity) && "Component already exists for this entity");

		EnsureIndexSize(entity);

		componentIndexes[entity] = static_cast<Entity>(components.size());
		components.push_back(std::move(component));
		ownerEntities.push_back(entity);
	}

	void RemoveFrom(Entity entity) override
	{
		assert(Has(entity) && "Removing a component the entity does not have");

		const Entity removedIndex = componentIndexes[entity];
		const Entity lastIndex = static_cast<Entity>(components.size() - 1);
		const Entity lastEntity = ownerEntities[lastIndex];

		components[removedIndex] = std::move(components[lastIndex]);
		ownerEntities[removedIndex] = lastEntity;
		componentIndexes[lastEntity] = removedIndex;

		components.pop_back();
		ownerEntities.pop_back();
		componentIndexes[entity] = INVALID_ENTITY;
	}

	bool Has(Entity entity) const override
	{
		return entity < componentIndexes.size()
			&& componentIndexes[entity] != INVALID_ENTITY
			&& componentIndexes[entity] < ownerEntities.size()
			&& ownerEntities[componentIndexes[entity]] == entity;
	}

	T& Get(Entity entity)
	{
		assert(Has(entity) && "Getting a component the entity does not have");

		return components[componentIndexes[entity]];
	}

	const T& Get(Entity entity) const
	{
		assert(Has(entity) && "Getting a component the entity does not have");

		return components[componentIndexes[entity]];
	}

	const std::vector<Entity>& Entities() const
	{
		return ownerEntities;
	}

private:
	void EnsureIndexSize(Entity entity)
	{
		if (entity >= componentIndexes.size())
			componentIndexes.resize(entity + 1, INVALID_ENTITY);
	}

	std::vector<T> components;
	std::vector<Entity> ownerEntities;
	std::vector<Entity> componentIndexes;
};