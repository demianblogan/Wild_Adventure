#pragma once

#include "core/ecs/Entity.h"

#include <nlohmann/json.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ECS
{
	class Registry;
}

class DataLoader
{
public:
	DataLoader();

	ECS::Entity LoadEntity(ECS::Registry& registry, const nlohmann::json& entityJson);
	ECS::Entity LoadEntityFromFile(ECS::Registry& registry, const std::string& path);
	
	ECS::Entity SpawnFromPrefab(ECS::Registry& registry, const std::string& path);

	std::vector<ECS::Entity> LoadScene(ECS::Registry& registry, const std::string& scenePath);
	std::vector<ECS::Entity> LoadSceneFromMap(ECS::Registry& registry, const std::string& mapPath);
	std::vector<ECS::Entity> LoadSceneFromMap(ECS::Registry& registry, const nlohmann::json& mapJson);

private:
	using ComponentLoader = std::function<void(ECS::Registry&, ECS::Entity, const nlohmann::json&)>;

	void RegisterLoaders();
	void AddImpliedComponents(ECS::Registry& registry, const std::vector<ECS::Entity>& entities);

	ECS::Entity LoadPrefabbedEntity(ECS::Registry& registry, const nlohmann::json& entry);

	// Parses a data file once and caches it: prefabs are re-spawned constantly
	// (every map object, box drop and death effect), and the files never change.
	const nlohmann::json& GetCachedJson(const std::string& path);

	std::unordered_map<std::string, ComponentLoader> loaders;
	std::unordered_map<std::string, nlohmann::json> fileCache;
};