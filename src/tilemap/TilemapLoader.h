#pragma once

#include "tilemap/Tilemap.h"

#include <string>

Tilemap LoadTilemap(const std::string& path, const std::string& textureName, int atlasWidthInTiles);