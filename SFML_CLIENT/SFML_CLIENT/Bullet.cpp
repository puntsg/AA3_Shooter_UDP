#include "Bullet.h"

Bullet::Bullet(sf::Vector2f startPos, sf::Vector2f dir)
	: direction(dir)
{
	GetTransform()->position = startPos;

	spriteRenderer = new SpriteRenderer(GetTransform());
	spriteRenderer->texture.loadFromFile("Sprites/Bullet.png");
	spriteRenderer->sprite.emplace(spriteRenderer->texture);
	spriteRenderer->sprite->setPosition(startPos);

	sf::FloatRect bounds = spriteRenderer->sprite->getLocalBounds();
	spriteRenderer->sprite->setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });

	if (dir.x < 0)
		spriteRenderer->sprite->setScale({ -1.f, 1.f });
	else
		spriteRenderer->sprite->setScale({ 1.f, 1.f });

	SetRenderer(spriteRenderer);
}

void Bullet::Update(float dt)
{
	GetTransform()->position += direction * speed * dt;
}
