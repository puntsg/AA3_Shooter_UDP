#pragma once
#include "Entity.h"
#include <vector>

class Tile : public Entity {
public:
	bool hasCollision = false;
};

class TileMap : public Entity
{
public:
	sf::Vector2i tileSize = sf::Vector2i(32, 32);
	std::vector<std::vector<Tile*>> tileGrid;

	~TileMap();
	void initMap(const std::string& mapDatafilePath);
	void render(sf::RenderWindow& window);
};

