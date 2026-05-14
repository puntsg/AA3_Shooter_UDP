#pragma once
#include "Scene.h"
#include <iostream>
#include <vector>
#include "..//Entities/Player/Player.h"
#include "../Entities/TileMap.h"
#include <SFML/Network.hpp>

class GameScene : public Scene
{
private:
    Player* p; 
    TileMap* t;
public:
    GameScene() = default;

    void OnEnter() override;

    void HandleEvent(const sf::Event& event) override;

    void Update(float dt) override;

    void Render(sf::RenderWindow& window) override;

    void OnExit() override;
};
