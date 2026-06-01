#pragma once

#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <string>
#include <mutex>
#include "ProtocolData.h"

#define MAX_HEALTH 5
#define MAX_LIFES 3
#define MAX_STRIKES 3
#define RESPAWN_X 400.f
#define RESPAWN_Y 200.f
#define CHEAT_THRESHOLD 80.f
#define PREDICT_TIMEOUT 0.2f

struct PlayerState
{
    sf::IpAddress ip = sf::IpAddress::Any;
    unsigned short port = 0;
    sf::Vector2f position;
    sf::Vector2f velocity;
    bool flipped = false;
    int health = MAX_HEALTH;
    int lifes = MAX_LIFES;
    bool ready = false;
    int cheatingStrikes = 0;
    sf::Clock lastPacketClock;
    int lastValidPacketId = 0;
};

class GameSession
{
public:
    GameSession(const std::string& roomId, const LobbyPlayerInfo& p1Info, const LobbyPlayerInfo& p2Info, sf::UdpSocket& socket, std::mutex& socketMutex);

    void ProcessMovePacket(int playerId, sf::Packet& packet);
    void ProcessShotPacket(int playerId, sf::Packet& packet);
    void ProcessTauntPacket(int playerId);
    void ProcessReadyPacket(int playerId);
    bool RegisterPlayerEndpoint(int playerId, const sf::IpAddress& ip, unsigned short port);

    void Update(float dt);

    bool IsFinished() const;
    std::string GetRoomId() const;

    bool BelongsToSession(const sf::IpAddress& ip, unsigned short port) const;
    int GetPlayerIdByAddress(const sf::IpAddress& ip, unsigned short port) const;

private:
    void BroadcastGameState();
    void SendToPlayer(int playerId, sf::Packet& packet);
    void SendToOther(int playerId, sf::Packet& packet);

    void HandleHit(int shooterPlayerId);
    void RespawnPlayer(int playerId);

    void PredictPositions(float dt);

    void FinishGame(int winnerPlayerId, bool cheating);

    PlayerState& GetState(int playerId);

    std::string roomId;
    int playerIds[2];
    PlayerState states[2];
    sf::UdpSocket& socket;
    std::mutex& socketMutex;  // mutex compartido para proteger udpSocket
    bool finished;
    bool bothReady;
    sf::Clock broadcastClock;
};
