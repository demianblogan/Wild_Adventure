#pragma once

#include "core/ecs/Entity.h"

#include <nlohmann/json.hpp>

#include <functional>
#include <string>
#include <unordered_map>

class Registry;

class DataLoader
{
public:
	DataLoader();

	Entity LoadEntity(Registry& registry, const nlohmann::json& entityJson);
	Entity LoadEntityFromFile(Registry& registry, const std::string& path);

private:
	using ComponentLoader = std::function<void(Registry&, Entity, const nlohmann::json&)>;

	void RegisterLoaders();

	std::unordered_map<std::string, ComponentLoader> loaders;
};