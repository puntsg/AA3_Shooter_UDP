#pragma once
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <optional>
#include "../../Scenes/SceneManager.h"
#include "../AnimatedRenderer.h"
#include "../Transform.h"

class Player: public Entity
{
public:
    int id = 0; 
    std::string nickName = "NoName";
    int scoreRanking = 1000;
    sf::Color color = sf::Color::White;
    bool isSpectator = false;
    bool isLocal = false;
    //https://gamefromscratch.com/sfml-c-tutorial-spritesheets-and-animation/
    Transform transform;
    sf::Texture texture;
    std::optional<sf::Sprite> sprite;


    Player();
    
    Player(int _id, std::string _name, int _score, sf::Color _color, bool _isLocal, Transform _transform, sf::Texture _texture, sf::Sprite _sprite);

    void Update(float dt) override;
};
