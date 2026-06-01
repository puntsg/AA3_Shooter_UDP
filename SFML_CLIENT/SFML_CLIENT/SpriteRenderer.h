#pragma once
#include "Renderer.h"
#include "SFML/Graphics.hpp"
class SpriteRenderer: public Renderer
{
public:
	sf::Texture texture;
	std::optional<sf::Sprite> sprite;
	bool flipped = false;

	SpriteRenderer(Transform* t) : Renderer(t) {}
	void render(sf::RenderWindow& window) override;
	void Update(float dt) override;
};

