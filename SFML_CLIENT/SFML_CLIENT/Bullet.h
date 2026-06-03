#pragma once
#include "Entity.h"
#include "SpriteRenderer.h"

class Bullet : public Entity
{
public:
    SpriteRenderer* spriteRenderer = nullptr;
    sf::Vector2f direction;
    float speed = 400.f;
    bool active = true;
    bool isLocal = true;   // true: la disparo el jugador local (golpea al rival); false: la disparo el rival (me golpea)

    Bullet(sf::Vector2f startPos, sf::Vector2f dir);
    void Update(float dt) override;
};

