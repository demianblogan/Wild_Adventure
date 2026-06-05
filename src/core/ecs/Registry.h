#pragma once

#include "core/ecs/ComponentPool.h"
#include "core/ecs/ComponentTypeId.h"
#include "core/ecs/Entity.h"
#include "core/ecs/IComponentPool.h"

#include <cassert>
#include <memory>
#include <vector>

namespace ECS
{
	class Registry
	{
	public:
		Entity CreateEntity()
		{
			return nextEntity++;
		}

		template <typename T>
		void Add(Entity entity, T component)
		{
			GetPool<T>().Add(entity, std::move(component));
		}

		template <typename T>
		void RemoveFrom(Entity entity)
		{
			GetPool<T>().RemoveFrom(entity);
		}

		void DestroyEntity(Entity entity)
		{
			for (std::unique_ptr<IComponentPool>& pool : pools)
			{
				if (pool != nullptr && pool->Has(entity))
					pool->RemoveFrom(entity);
			}
		}

		template <typename T>
		bool Has(Entity entity)
		{
			return GetPool<T>().Has(entity);
		}

		template <typename T>
		T& Get(Entity entity)
		{
			return GetPool<T>().Get(entity);
		}

		template <typename FirstComponent, typename... RestComponents, typename Function>
		void ForEach(Function function)
		{
			auto& firstPool = GetPool<FirstComponent>();

			for (Entity entity : firstPool.Entities())
			{
				if ((Has<RestComponents>(entity) && ...))
					function(entity, Get<FirstComponent>(entity), Get<RestComponents>(entity)...);
			}
		}

	private:
		template <typename T>
		ComponentPool<T>& GetPool()
		{
			const std::size_t typeId = GetComponentTypeId<T>();

			if (typeId >= pools.size())
				pools.resize(typeId + 1);

			if (!pools[typeId])
				pools[typeId] = std::make_unique<ComponentPool<T>>();

			IComponentPool* basePool = pools[typeId].get();
			return *static_cast<ComponentPool<T>*>(basePool);
		}

		Entity nextEntity = 0;
		std::vector<std::unique_ptr<IComponentPool>> pools;
	};
}