#pragma once

#include "core/ecs/Entity.h"

#include <nlohmann/json.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class Registry;

class DataLoader
{
public:
	DataLoader();

	Entity LoadEntity(Registry& registry, const nlohmann::json& entityJson);
	Entity LoadEntityFromFile(Registry& registry, const std::string& path);

	std::vector<Entity> LoadScene(Registry& registry, const std::string& scenePath);

private:
	using ComponentLoader = std::function<void(Registry&, Entity, const nlohmann::json&)>;

	void RegisterLoaders();
	void AddImpliedComponents(Registry& registry, const std::vector<Entity>& entities);

	std::unordered_map<std::string, ComponentLoader> loaders;
};