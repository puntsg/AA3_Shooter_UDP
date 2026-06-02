#pragma once
#include "Scene.h"
#include "Player.h"
#include "TileMap.h"
#include "Bullet.h"
#include <SFML/Audio.hpp>
#include <vector>
#include <optional>
#include <string>

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
    void SendTaunt();
    void PlayTaunt(int taunterId);
    void ResolveCollisions(Player* p);
    void HandleGameEnd();

    Player*              localPlayer   = nullptr;
    Player*              remotePlayer  = nullptr;
    TileMap*             tileMap       = nullptr;
    std::vector<Bullet*> bullets;

    float m_sendTimer     = 0.f;
    int   m_packetSeqId   = 0;
    float m_gameOverTimer = 0.f;
    float m_tauntLock     = 0.f;
    float m_tauntCooldown = 0.f;
    float m_tauntTextTime = 0.f;

    sf::SoundBuffer          m_tauntBuffer;
    std::optional<sf::Sound> m_tauntSound;
    sf::Font                 m_font;
    bool                     m_soundLoaded = false;
    bool                     m_fontLoaded = false;
    std::string              m_tauntText;

    static constexpr float SEND_INTERVAL = 0.033f;
    static constexpr float TAUNT_LOCK_TIME = 0.8f;
    static constexpr float TAUNT_COOLDOWN = 1.2f;
    static constexpr float TAUNT_TEXT_TIME = 1.4f;
    static constexpr const char* TAUNT_SOUND = "assets/Burla.mp3";
};
