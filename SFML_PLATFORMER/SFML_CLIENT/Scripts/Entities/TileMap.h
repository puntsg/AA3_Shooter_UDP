#pragma once
#include "Entity.h"

class Tile : public Entity {
	bool hasCollision;
};
class TileMap: public Entity
{
public:
	sf::Vector2i tileSize = sf::Vector2i(32,32);
	//Tile* tileGrid[][];
	void initMap(std::string  mapDatafilePath);
};

