#include "TileMap.h"
#include "SpriteRenderer.h"
#include <iostream>
#include <fstream>

TileMap::~TileMap()
{
	for (std::vector<Tile*>& row : tileGrid)
		for (Tile* tile : row)
			delete tile;
}

void TileMap::initMap(const std::string& mapDatafilePath)
{
	std::ifstream file(mapDatafilePath);
	if (!file.is_open()) {
		std::cout << "Error: no se pudo abrir " << mapDatafilePath << "\n";
		return;
	}

	sf::Vector2f currentPos = sf::Vector2f(0, 0);
	std::string line;
	while (std::getline(file, line)) {
		std::vector<Tile*> gridRow;
		for (char c : line) {
			Tile* tile = new Tile();
			tile->GetTransform()->position = currentPos;
			if (c == '#') {
				tile->hasCollision = true;
				SpriteRenderer* r = new SpriteRenderer(tile->GetTransform());
				if (r->texture.loadFromFile("Sprites/Tiles/Dirt.png"))
				{
					r->sprite.emplace(r->texture);
					r->sprite->setPosition(tile->GetTransform()->position);
					tile->SetRenderer(r);
				}
				else
				{
					delete r;
				}
			}

			gridRow.push_back(tile);
			currentPos.x += tileSize.x;
		}
		tileGrid.push_back(gridRow);
		currentPos.x = 0;
		currentPos.y += tileSize.y;
	}
	file.close();
	std::cout << "Mapa generado\n";
}

void TileMap::render(sf::RenderWindow& window)
{
	for (std::vector<Tile*>& row : tileGrid)
		for (Tile* tile : row)
			if (tile->GetRenderer())
				tile->GetRenderer()->render(window);
}
