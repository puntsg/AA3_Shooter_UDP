#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include "ProtocolData.h"

struct PlayerVisual
{
    sf::Vector2f       position;
    sf::Vector2f       velocity;
    bool               flipped   = false;
    int                health    = 5;
    int                lifes     = 3;
    bool               onGround  = false;
    int                dbId      = -1;
    int                localId   = -1;
    std::string        nickname;
    sf::RectangleShape shape;
};

struct Bullet
{
    sf::Vector2f position;
    float        direction = 1.f;  // 1 = derecha, -1 = izquierda
    float        traveled  = 0.f;
    bool         active    = true;
    bool         fromLocal = true;
};

class GameManager
{
public:
    GameManager();

    void InitGame(const std::vector<LobbyPlayerInfo>& players, int localDbId);

    void HandleInput(const sf::Event& event);

    void Update(float dt);

    void DrawGame(sf::RenderWindow& window);

    void DrawHUD(sf::RenderWindow& window);

    // Aplicar datos recibidos del GameServer por UDP
    void ApplyTransform(const TransformData& data);
    void ApplyHit(const PlayerHitData& data);
    void ApplyTaunt(int taunterDbId);
    void SpawnRivalBullet(const ShootReplicateData& data);
    void ApplyEndgame(const EndgameData& data);

    bool IsGameOver()          const;
    bool IsLocalPlayerWinner() const;

    void Reset();

private:
    void LoadMap();
    void ApplyPhysics(PlayerVisual& player, float dt);
    void ResolveCollisions(PlayerVisual& player);
    void UpdateBullets(float dt);

    void SendTransform();
    void SendShoot();
    void SendTaunt();

    // Mapa
    std::vector<std::string>        m_mapRows;
    std::vector<sf::RectangleShape> m_tiles;
    int                             m_mapColCount = 0;
    int                             m_mapRowCount = 0;

    // Jugadores
    PlayerVisual m_local;
    PlayerVisual m_rival;
    int          m_localDbId = -1;
    int          m_myLocalId = -1;  

    // Balas
    std::vector<Bullet> m_bullets;

    // Input
    bool m_leftHeld  = false;
    bool m_rightHeld = false;

    // Timer de envio de red
    float m_sendTimer   = 0.f;
    int   m_packetSeqId = 0;

    // Estado de partida
    bool m_gameOver = false;
    bool m_localWon = false;

    // Taunt
    bool        m_showTaunt  = false;
    float       m_tauntTimer = 0.f;
    std::string m_tauntMsg;

    // Fuente para HUD y textos
    sf::Font m_font;

    // Audio
    sf::SoundBuffer m_tauntBuffer;
    sf::Sound       m_tauntSound;
    bool            m_soundLoaded = false;

    // Constantes de juego
    static constexpr float TILE_SIZE        = 50.f;
    static constexpr float MAP_OFFSET_Y     = 60.f;   
    static constexpr float PLAYER_W         = 32.f;
    static constexpr float PLAYER_H         = 48.f;
    static constexpr float PLAYER_SPEED     = 200.f;
    static constexpr float JUMP_FORCE       = -520.f;
    static constexpr float GRAVITY          = 980.f;
    static constexpr float BULLET_SPEED     = 500.f;
    static constexpr float BULLET_W         = 10.f;
    static constexpr float BULLET_H         = 6.f;
    static constexpr float BULLET_MAX_DIST  = 700.f;
    static constexpr float SEND_INTERVAL    = 0.033f; 
    static constexpr float SNAP_THRESHOLD   = 100.f;  
    static constexpr float TAUNT_DURATION   = 2.f;
    static constexpr int   MAX_HEALTH       = 5;
    static constexpr int   MAX_LIFES        = 3;
    static constexpr const char* VERSION_FILE = "maps/map_version.txt";
    static constexpr const char* FALLBACK_MAP = "maps/map_v1.txt";
    static constexpr const char* TAUNT_SOUND  = "assets/taunt.wav";
};
