#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.h"
class UIElement: public Entity
{
public :
	virtual void handleEvent(const sf::Event& event) = 0;
	virtual void Draw(sf::RenderWindow& window) =  0;
};

