#pragma once
#include "Scene.h"
#include "GameManager.h"
#include <iostream>
#include <vector>
#include <SFML/Network.hpp>
#include "NetworkManager.h"

class GameScene : public Scene
{
private:
    GameManager gameManager;

public:
    GameScene() = default;

    void SetupGame(const std::vector<Player>& players, int localID);

    void SyncNextTurn(int playerID);

    void OnEnter() override;

    void HandleEvent(const sf::Event& event) override;

    void Update(float dt) override;

    void Render(sf::RenderWindow& window) override;

    void OnExit() override;
};
