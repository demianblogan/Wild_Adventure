#pragma once

#include "core/ResourceManager.h"

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <nlohmann/json.hpp>

#include <string>
#include <unordered_map>

struct Resources
{
	void LoadTexturesFromFile(const std::string& path);

	// Parsed .tmj maps, cached for the whole session: parsing a big level is the
	// slowest part of a level load, and restarts reload the same unchanged file.
	const nlohmann::json& GetMapJson(const std::string& path);

	ResourceManager<sf::Texture> textures;
	ResourceManager<sf::Font> fonts;
	ResourceManager<sf::SoundBuffer> sounds;
	ResourceManager<sf::Music> music;
	ResourceManager<sf::Shader> shaders;

	std::unordered_map<std::string, nlohmann::json> mapJsonCache;
};