#pragma once
#include "Scene.h"
#include "Player.h"
#include "TileMap.h"
#include "Bullet.h"
#include <vector>

class GameScene : public Scene
{
public:
    GameScene() = default;

    void OnEnter()                           override;
    void HandleEvent(const sf::Event& event) override;
    void Update(float dt)                    override;
    void Render(sf::RenderWindow& window)    override;
    void OnExit()                            override;

private:
    void SendTransform();
    void ResolveCollisions(Player* p);
    void HandleGameEnd();

    Player*              localPlayer   = nullptr;
    Player*              remotePlayer  = nullptr;
    TileMap*             tileMap       = nullptr;
    std::vector<Bullet*> bullets;

    float m_sendTimer     = 0.f;
    int   m_packetSeqId   = 0;
    float m_gameOverTimer = 0.f;

    static constexpr float SEND_INTERVAL = 0.033f;
};
