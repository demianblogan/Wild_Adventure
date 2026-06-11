#pragma once

#include "tilemap/Tilemap.h"

#include <nlohmann/json.hpp>

#include <string>

Tilemap LoadTilemap(const nlohmann::json& data, const std::string& textureName, int atlasWidthInTiles);
Tilemap LoadTilemap(const std::string& path, const std::string& textureName, int atlasWidthInTiles);