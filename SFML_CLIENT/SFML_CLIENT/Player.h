#pragma once
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "SceneManager.h"

class Player
{
public:
    int id = 0; 
    std::string nickName = "NoName";
    int scoreRanking = 1000;
    sf::Color color = sf::Color::White;
    bool isSpectator = false;
    bool isLocal = false;

    Player() = default;
    
    Player(int _id, std::string _name, int _score, sf::Color _color, bool _isLocal)
        : id(_id), nickName(_name), scoreRanking(_score), color(_color), isLocal(_isLocal) {}
};
