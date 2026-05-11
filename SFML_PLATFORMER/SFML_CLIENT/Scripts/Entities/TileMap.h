#pragma once
#include "Entity.h"

class Tile : public Entity {
	bool hasCollision;
};
class TileMap: public Entity
{
	sf::Vector2i tileSize;
	//Tile* tileGrid[][];
	void initMap(std::string  mapDatafilePath);
};

