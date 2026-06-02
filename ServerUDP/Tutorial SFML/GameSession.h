#pragma once

#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <string>
#include <mutex>
#include "ProtocolData.h"

#define MAX_HEALTH 5
#define MAX_LIFES 3
#define MAX_STRIKES 3
#define P1_START_X 160.f
#define P2_START_X 320.f
#define START_Y 240.f
#define RESPAWN_X 240.f
#define RESPAWN_Y 240.f
#define CHEAT_THRESHOLD 220.f
#define PREDICT_TIMEOUT 0.2f
#define DISCONNECT_TIMEOUT 3.f
#define HELLO_TIMEOUT 8.f

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
    bool disconnected = false;
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
    void DisconnectPlayer(int playerId);

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
    void CheckDisconnects();
    void CheckHelloTimeout();
    void SendPlayerDisconnected(int playerId);

    void FinishGame(int winnerPlayerId, bool cheating);

    PlayerState& GetState(int playerId);
    int GetIndex(int playerId) const;
    int GetOtherPlayerId(int playerId) const;

    std::string roomId;
    int playerIds[2];
    PlayerState states[2];
    sf::UdpSocket& socket;
    std::mutex& socketMutex;  // mutex compartido para proteger udpSocket
    bool finished;
    bool bothReady;
    sf::Clock broadcastClock;
    sf::Clock sessionClock;
};
