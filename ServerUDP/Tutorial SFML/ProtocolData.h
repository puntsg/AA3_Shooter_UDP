#pragma once

#include <SFML/Network.hpp>
#include <string>
#include <vector>
#include "PacketTypes.h"

inline sf::Packet& operator<<(sf::Packet& packet, const sf::Vector2f& v)
{
    return packet << v.x << v.y;
}
inline sf::Packet& operator>>(sf::Packet& packet, sf::Vector2f& v)
{
    return packet >> v.x >> v.y;
}

struct LobbyPlayerInfo
{
    int playerId       = -1;
    std::string username;
    std::string ip;
    bool isHost        = false;
};

inline sf::Packet& operator<<(sf::Packet& packet, const LobbyPlayerInfo& data)
{
    packet << data.playerId << data.username << data.ip << data.isHost;
    return packet;
}
inline sf::Packet& operator>>(sf::Packet& packet, LobbyPlayerInfo& data)
{
    packet >> data.playerId >> data.username >> data.ip >> data.isHost;
    return packet;
}

// sala que llega desde matchmaking
struct SessionStartData
{
    std::string roomId;
    int playerCount = 0;
    std::vector<LobbyPlayerInfo> players;
};

struct SessionStartResponseData
{
    bool success = false;
    std::string roomId;
    std::string message;
};

// hello del cliente
struct UdpHelloData
{
    std::string roomId;
    int playerId = -1;
};

sf::Packet& operator<<(sf::Packet& packet, const SessionStartData& data);
sf::Packet& operator>>(sf::Packet& packet, SessionStartData& data);
sf::Packet& operator<<(sf::Packet& packet, const SessionStartResponseData& data);
sf::Packet& operator>>(sf::Packet& packet, SessionStartResponseData& data);


struct TransformData
{
    int packetId = 0;      // ID secuencial del paquete para reconciliacion
    int dbId = -1;
    int localPlayerId = -1;
    float x = 0.f;
    float y = 0.f;
    bool flipped = false;
    float spriteStartX = 0.f, spriteStartY = 0.f, spriteEndX = 0.f, spriteEndY = 0.f;
};

inline sf::Packet& operator<<(sf::Packet& packet, const TransformData& data)
{
    packet << data.packetId << data.dbId << data.localPlayerId << data.x << data.y << data.flipped
        << data.spriteStartX << data.spriteStartY << data.spriteEndX << data.spriteEndY;
    return packet;
}
inline sf::Packet& operator>>(sf::Packet& packet, TransformData& data)
{
    packet >> data.packetId >> data.dbId >> data.localPlayerId >> data.x >> data.y >> data.flipped
        >> data.spriteStartX >> data.spriteStartY >> data.spriteEndX >> data.spriteEndY;
    return packet;
}

// instanciar la bullet
struct ShootReplicateData
{
    sf::Vector2f position;
    bool flipped = false;
};

inline sf::Packet& operator<<(sf::Packet& packet, const ShootReplicateData& data)
{
    packet << data.position << data.flipped;
    return packet;
}
inline sf::Packet& operator>>(sf::Packet& packet, ShootReplicateData& data)
{
    packet >> data.position >> data.flipped;
    return packet;
}

struct PlayerHitData
{
    int targetPlayerId = -1;
    int newHealth      = 0;
    int newLifes       = 0;
    sf::Vector2f respawnPosition;
};

inline sf::Packet& operator<<(sf::Packet& packet, const PlayerHitData& data)
{
    packet << data.targetPlayerId << data.newHealth << data.newLifes << data.respawnPosition;
    return packet;
}
inline sf::Packet& operator>>(sf::Packet& packet, PlayerHitData& data)
{
    packet >> data.targetPlayerId >> data.newHealth >> data.newLifes >> data.respawnPosition;
    return packet;
}

struct EndgameData
{
    int winnerPlayerId = -1;
    int loserPlayerId  = -1;
    bool cheating      = false;
};

inline sf::Packet& operator<<(sf::Packet& packet, const EndgameData& data)
{
    packet << data.winnerPlayerId << data.loserPlayerId << data.cheating;
    return packet;
}
inline sf::Packet& operator>>(sf::Packet& packet, EndgameData& data)
{
    packet >> data.winnerPlayerId >> data.loserPlayerId >> data.cheating;
    return packet;
}

struct Result
{
    std::string username;
    int scoredPoints = 0;
};

struct GameResultData
{
    std::string roomId;
    std::vector<Result> results;
};

sf::Packet& operator<<(sf::Packet& packet, const Result& data);
sf::Packet& operator>>(sf::Packet& packet, Result& data);
sf::Packet& operator<<(sf::Packet& packet, const GameResultData& data);
sf::Packet& operator>>(sf::Packet& packet, GameResultData& data);

inline sf::Packet& operator<<(sf::Packet& packet, const UdpHelloData& data)
{
    packet << data.roomId << data.playerId;
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, UdpHelloData& data)
{
    packet >> data.roomId >> data.playerId;
    return packet;
}
