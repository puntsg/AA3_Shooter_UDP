#pragma once
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "../../Scenes/SceneManager.h"
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
    sf::Sprite sprite;


    Player() = default;
    
    Player(int _id, std::string _name, int _score, sf::Color _color, bool _isLocal, Transform _transform, sf::Texture _texture,sf::Sprite _sprite)
        : id(_id), nickName(_name), scoreRanking(_score), color(_color), isLocal(_isLocal), transform(_transform),texture(_texture),sprite(_sprite) {}

    void Update(float dt) override;
};
