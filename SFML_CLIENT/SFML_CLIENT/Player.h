#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Bullet.h"
#include "AnimatedRenderer.h"

class Player :public Entity
{
public:
    int id = 0; 
    std::string nickName = "NoName";
    int scoreRanking = 1000;
    sf::Color color = sf::Color::White;
    bool isSpectator = false;
    bool isLocal = false;

    Player();
    Player(int _id, std::string _name, int _score, sf::Color _color, bool _isLocal);

    void Update(float dt) override;

    AnimatedRenderer* animRenderer = nullptr;

    bool grounded = false;
    sf::Vector2f velocity;
    Bullet* pendingBullet = nullptr;
    float fireRate = 0.3f;
private:
    float fireCooldown = 0.f;
};
