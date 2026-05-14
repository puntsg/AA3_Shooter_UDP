#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "../../Scenes/SceneManager.h"
#include "../AnimatedRenderer.h"

class Player : public Entity
{
public:
    int id = 0;
    std::string nickName = "NoName";
    int scoreRanking = 1000;
    sf::Color color = sf::Color::White;
    bool isSpectator = false;
    bool isLocal = false;
    
    AnimatedRenderer* animRenderer = nullptr;

    Player();
    Player(int _id, std::string _name, int _score, sf::Color _color, bool _isLocal);

    void Update(float dt) override;

    bool grounded = true;
    sf::Vector2f velocity;
};
