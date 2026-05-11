#include "SpriteRenderer.h"


void SpriteRenderer::render(sf::RenderWindow& window)
{
	if (sprite.has_value()) {
		sprite->setPosition(transform->position);
		window.draw(*sprite);
	}
}

void SpriteRenderer::Update(float dt)
{
}
