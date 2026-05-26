#include "SpriteRenderer.h"


void SpriteRenderer::render(sf::RenderWindow& window)
{
	if (sprite.has_value()) {
		sprite->setPosition(transform->position);
		if (flipped)
			sprite->setScale({ -sprite->getScale().x,sprite->getScale().y});
		window.draw(*sprite);
	}
}

void SpriteRenderer::Update(float dt)
{
}
