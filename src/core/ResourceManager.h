#pragma once

#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics/Font.hpp>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

template <typename Resource>
class ResourceManager
{
public:
	template <typename... Args>
	void Load(const std::string& id, const std::filesystem::path& filepath, Args&&... args)
	{
		Resource resource;
		bool isLoaded = false;

		if constexpr (std::is_same_v<Resource, sf::Music> || std::is_same_v<Resource, sf::Font>)
			isLoaded = resource.openFromFile(filepath, std::forward<Args>(args)...);
		else
			isLoaded = resource.loadFromFile(filepath, std::forward<Args>(args)...);

		if (!isLoaded)
			throw std::runtime_error("Failed to load resource: " + filepath.string());

		const auto [iterator, isInserted] = resources.emplace(id, std::move(resource));
		if (!isInserted)
			throw std::runtime_error("Resource already loaded: " + id);
	}

	Resource& Get(const std::string& id)
	{
		const auto iterator = resources.find(id);
		if (iterator == resources.end())
			throw std::runtime_error("Requested resource was not loaded: " + id);

		Resource& resource = iterator->second;
		return resource;
	}

	const Resource& Get(const std::string& id) const
	{
		const auto iterator = resources.find(id);
		if (iterator == resources.end())
			throw std::runtime_error("Requested resource was not loaded: " + id);

		Resource& resource = iterator->second;
		return resource;
	}

private:
	std::unordered_map<std::string, Resource> resources;
};