#pragma once

#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <string>
#include <vector>
#include <mutex>
#include "ProtocolData.h"

static const int MAX_HEALTH = 5;
static const int MAX_LIFES = 3;
static const int MAX_STRIKES = 3;
static const float P1_START_X = 96.f;    // = spawn del cliente (Config::Gameplay::P1_SPAWN)
static const float P2_START_X = 576.f;   // = spawn del cliente (Config::Gameplay::P2_SPAWN)
static const float START_Y = 256.f;
static const float RESPAWN_X = 336.f;     // centro entre ambos spawns
static const float RESPAWN_Y = 256.f;
static const float CHEAT_THRESHOLD = 220.f;
static const float PREDICT_TIMEOUT = 0.2f;
static const float DISCONNECT_TIMEOUT = 3.f;
static const float HELLO_TIMEOUT = 8.f;
static const float BULLET_SPEED = 400.f;
static const float BULLET_MAX_DIST = 1200.f;

struct PlayerState
{
    sf::IpAddress ip = sf::IpAddress::Any;
    unsigned short port = 0;
    sf::Vector2f position;
    sf::Vector2f velocity;
    bool flipped = false;
    float spriteStartX = 0.f, spriteStartY = 0.f, spriteEndX = 0.f, spriteEndY = 0.f;
    int health = MAX_HEALTH;
    int lifes = MAX_LIFES;
    bool ready = false;
    int cheatingStrikes = 0;
    sf::Clock lastPacketClock;
    int lastValidPacketId = 0;
    std::vector<int> processedCriticalPackets;
    bool disconnected = false;
};

struct BulletState
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    bool flipped = false;
    int ownerID = 0;
    float traveled = 0.f;
    bool active = true;
};

struct PendingCriticalPacket
{
    int packetId = 0;
    int targetPlayerId = -1;
    sf::Packet packet;
    float nextSendTime = 0.f;
    int attempts = 0;
};

class GameSession
{
public:
    GameSession(const std::string& roomId, const LobbyPlayerInfo& p1Info, const LobbyPlayerInfo& p2Info, sf::UdpSocket& socket, std::mutex& socketMutex);

    void ProcessMovePacket(int playerId, sf::Packet& packet);
    void ProcessShotPacket(int playerId, sf::Packet& packet);
    void ProcessTauntPacket(int playerId, sf::Packet& packet);
    void ProcessCriticalAckPacket(int playerId, sf::Packet& packet);
    bool RegisterPlayerEndpoint(int playerId, const sf::IpAddress& ip, unsigned short port);
    void DisconnectPlayer(int playerId);

    void Update(float dt);

    bool IsFinished() const;

    bool BelongsToSession(const sf::IpAddress& ip, unsigned short port) const;
    int GetPlayerIdByAddress(const sf::IpAddress& ip, unsigned short port) const;

private:
    void BroadcastGameState();
    void SendToOther(int playerId, sf::Packet& packet);
    void SendCriticalToPlayer(int playerId, sf::Packet& packet, int packetId, const char* context);
    void SendCriticalAck(int playerId, int packetId);
    void UpdateCriticalPackets();
    bool StoreProcessedCriticalPacket(int playerId, int packetId);
    int CreateCriticalPacketId();

    void UpdateBullets(float dt);
    std::vector<BulletState> bullets;
    std::mutex bulletsMutex;
    std::vector<PendingCriticalPacket> pendingCriticalPackets;
    std::mutex criticalPacketsMutex;

    void LoadCollisionMap();
    bool IsWallAt(const sf::Vector2f& position) const;
    bool SegmentHitsWall(const sf::Vector2f& from, const sf::Vector2f& to) const;
    bool PointHitsPlayer(const sf::Vector2f& point, int targetPlayerId) const;
    bool SegmentHitsPlayer(const sf::Vector2f& from, const sf::Vector2f& to, int targetPlayerId) const;
    bool ClampPositionToMapBottom(sf::Vector2f& position) const;
    void HandleHit(int shooterPlayerId);
    void RespawnPlayer(int playerId);

    void PredictPositions(float dt);
    void CheckDisconnects();
    void CheckHelloTimeout();
    void SendPlayerDisconnected(int playerId);

    void FinishGame(int winnerPlayerId, bool cheating);
    void ReportGameResult(int winnerPlayerId, int loserPlayerId);

    PlayerState& GetState(int playerId);
    int GetIndex(int playerId) const;
    int GetOtherPlayerId(int playerId) const;

    std::string roomId;
    int playerIds[2];
    std::string playerNames[2];
    PlayerState states[2];
    std::vector<std::string> mapRows;
    sf::UdpSocket& socket;
    std::mutex& socketMutex;  // mutex compartido para proteger udpSocket
    bool finished;
    bool bothReady;
    sf::Clock broadcastClock;
    sf::Clock sessionClock;
    sf::Clock criticalClock;
    int nextCriticalPacketId;
};
