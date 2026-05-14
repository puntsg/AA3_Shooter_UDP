#pragma once
#include "Entity.h"
#include "SpriteRenderer.h"
class Bullet :
    public Entity
{
public:
    SpriteRenderer* renderer;
    sf::Vector2f directionVector;
    Bullet();
    void Update(float dt) override;
};

