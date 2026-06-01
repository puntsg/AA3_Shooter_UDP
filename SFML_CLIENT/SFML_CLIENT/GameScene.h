#pragma once
#include "Scene.h"
#include "GameManager.h"

class GameScene : public Scene
{
public:
    GameScene() = default;

    void OnEnter()                          override;
    void HandleEvent(const sf::Event& event) override;
    void Update(float dt)                   override;
    void Render(sf::RenderWindow& window)   override;
    void OnExit()                           override;

private:
    // Gestiona el fin de partida
    void HandleGameEnd();

    GameManager m_gameManager;
    float       m_gameOverTimer = 0.f;
};
