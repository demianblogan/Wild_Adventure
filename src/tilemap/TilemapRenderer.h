#pragma once

namespace sf
{
	class RenderTarget;
}

struct Tilemap;
struct Resources;

void DrawTilemap(const Tilemap& tilemap, sf::RenderTarget& target, Resources& resources);