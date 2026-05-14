#include "Bullet.h"

Bullet::Bullet()
{
	renderer = new SpriteRenderer(GetTransform());
	renderer->texture.loadFromFile("Sprites/Bullet.png");
	renderer->sprite.emplace(renderer->texture);
}

void Bullet::Update(float dt)
{
	GetTransform()->position = sf::Vector2(
		GetTransform()->position.x + (dt*directionVector.x),
		GetTransform()->position.y + (dt*directionVector.y)
	);
}
