#include "TilemapRenderer.h"

#include "core/Resources.h"
#include "tilemap/Tilemap.h"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

void DrawTilemap(const Tilemap& tilemap, sf::RenderTarget& target, Resources& resources)
{
	const sf::Texture& texture = resources.textures.Get(tilemap.textureName);

	sf::RenderStates states;
	states.texture = &texture;

	for (const Tilemap::Layer& layer : tilemap.layers)
		target.draw(layer.vertices, states);
}