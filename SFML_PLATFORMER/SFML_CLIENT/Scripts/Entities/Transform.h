#pragma once
#include <SFML/System/Vector2.hpp>

class Transform {
public:
	sf::Vector2f position;
	float rotation;
	sf::Vector2f scale;

	Transform() : position(0.0f, 0.0f), rotation(0.0f), scale(1.0f, 1.0f) {}

	Transform(const sf::Vector2f& pos, float rot, const sf::Vector2f& sca)
		: position(pos), rotation(rot), scale(sca) {}
};