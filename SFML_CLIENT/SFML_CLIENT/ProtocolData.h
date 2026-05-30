#pragma once

#include <SFML/Network.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>
#include "PacketTypes.h"

// Serializacion
inline sf::Packet& operator<<(sf::Packet& packet, const sf::Vector2f& v)
{
    return packet << v.x << v.y;
}
inline sf::Packet& operator>>(sf::Packet& packet, sf::Vector2f& v)
{
    return packet >> v.x >> v.y;
}

struct UdpHelloData
{
    std::string roomId;
    int playerId = -1;
};
inline sf::Packet& operator<<(sf::Packet& packet, const UdpHelloData& data)
{
    return packet << data.roomId << data.playerId;
}
inline sf::Packet& operator>>(sf::Packet& packet, UdpHelloData& data)
{
    return packet >> data.roomId >> data.playerId;
}

// Pos y orientacion
struct TransformData
{
    int packetId      = 0;
    int dbId          = -1;
    int localPlayerId = -1;
    float x           = 0.f;
    float y           = 0.f;
    bool flipped      = false;
};
inline sf::Packet& operator<<(sf::Packet& packet, const TransformData& data)
{
    return packet << data.packetId << data.dbId << data.localPlayerId << data.x << data.y << data.flipped;
}
inline sf::Packet& operator>>(sf::Packet& packet, TransformData& data)
{
    return packet >> data.packetId >> data.dbId >> data.localPlayerId >> data.x >> data.y >> data.flipped;
}

// Disparo replicado desde enemigo
struct ShootReplicateData
{
    sf::Vector2f position;
    bool flipped = false;
};
inline sf::Packet& operator<<(sf::Packet& packet, const ShootReplicateData& data)
{
    return packet << data.position << data.flipped;
}
inline sf::Packet& operator>>(sf::Packet& packet, ShootReplicateData& data)
{
    return packet >> data.position >> data.flipped;
}

struct PlayerHitData
{
    int targetPlayerId  = -1;
    int newHealth       = 0;
    int newLifes        = 0;
    sf::Vector2f respawnPosition;
};
inline sf::Packet& operator<<(sf::Packet& packet, const PlayerHitData& data)
{
    return packet << data.targetPlayerId << data.newHealth << data.newLifes << data.respawnPosition;
}
inline sf::Packet& operator>>(sf::Packet& packet, PlayerHitData& data)
{
    return packet >> data.targetPlayerId >> data.newHealth >> data.newLifes >> data.respawnPosition;
}

struct EndgameData
{
    int winnerPlayerId = -1;
    int loserPlayerId  = -1;
    bool cheating      = false;
};
inline sf::Packet& operator<<(sf::Packet& packet, const EndgameData& data)
{
    return packet << data.winnerPlayerId << data.loserPlayerId << data.cheating;
}
inline sf::Packet& operator>>(sf::Packet& packet, EndgameData& data)
{
    return packet >> data.winnerPlayerId >> data.loserPlayerId >> data.cheating;
}

//Ranking
struct RankingData
{
    std::string playerName;
    int score ;
};

struct RankingRequestData
{
    std::string username;  
};

struct RankingResponseData
{
    bool success = false;
    std::string message;
    std::vector<RankingData> entries;
};

inline sf::Packet& operator<<(sf::Packet& packet, const RankingData& data)
{
    packet << data.playerName << data.score;
    return packet;
}
inline sf::Packet& operator>>(sf::Packet& packet, RankingData& data)
{
    packet >> data.playerName >> data.score;
    return packet;
}
inline sf::Packet& operator<<(sf::Packet& packet, const RankingRequestData& data)
{
    packet << data.username;
    return packet;
}
inline sf::Packet& operator>>(sf::Packet& packet, RankingRequestData& data)
{
    packet >> data.username;
    return packet;
}
sf::Packet& operator<<(sf::Packet& packet, const RankingResponseData& data);
sf::Packet& operator>>(sf::Packet& packet, RankingResponseData& data);

// Datos de Login
struct LoginRequestData
{
    std::string username;
    std::string password;
};

struct LoginResponseData
{
    bool success = false;
    int playerId = -1;
    std::string username;
    int score = 0;
    std::string message;
};

// Datos de Registro
struct RegisterRequestData
{
    std::string username;
    std::string password;
};

struct RegisterResponseData
{
    bool success = false;
    std::string message;
};

// Datos de Crear Sala
struct CreateRoomRequestData
{
    std::string roomId;
    std::string username;
    unsigned short gamePort = 0;
};

struct CreateRoomResponseData
{
    bool success = false;
    std::string roomId;
    std::string message;
};

// Datos de Unirse a Sala
struct JoinRoomRequestData
{
    std::string roomId;
    std::string username;
    unsigned short gamePort = 0;
};

struct JoinRoomResponseData
{
    bool success = false;
    std::string roomId;
    std::string message;
};

// Datos de Jugador dentro del Lobby
struct LobbyPlayerInfo
{
    int playerId = -1;
    std::string username;
    std::string ip;
    unsigned short gamePort = 0;
    bool isHost = false;
};

// Estado de la Sala
struct RoomStatusUpdateData
{
    std::string roomId;
    int currentPlayers = 0;
    int maxPlayers = 0;
    std::vector<LobbyPlayerInfo> players;
};

// Inicio de partida
struct StartGameData
{
    std::string roomId;
    int playerCount = 0;
    std::string gameServerIp;
    unsigned short gameServerUdpPort = 0;
    std::vector<LobbyPlayerInfo> players;
};

struct Result {
    std::string username;
    int scoredPoints = 0;
};
struct GameResultData {
    std::vector<Result> results;
};
sf::Packet& operator<<(sf::Packet& packet, const Result& data);
sf::Packet& operator>>(sf::Packet& packet, Result& data);
sf::Packet& operator<<(sf::Packet& packet, const GameResultData& data);
sf::Packet& operator>>(sf::Packet& packet, GameResultData& data);

// Error generico
struct ErrorMessageData
{
    std::string message;
};

struct RankingUpdateData
{
    std::string roomId;
    std::vector<int> placementOrder; // orden de jugadores
};

// hello udp
struct UdpHelloData
{
    std::string roomId;
    int playerId = -1;
};

// LobbyPlayerInfo
inline sf::Packet& operator<<(sf::Packet& packet, const LobbyPlayerInfo& data)
{
    packet << data.playerId
        << data.username
        << data.ip
        << data.gamePort
        << data.isHost;
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, LobbyPlayerInfo& data)
{
    packet >> data.playerId
        >> data.username
        >> data.ip
        >> data.gamePort
        >> data.isHost;
    return packet;
}

// LoginRequestData
inline sf::Packet& operator<<(sf::Packet& packet, const LoginRequestData& data)
{
    packet << data.username << data.password;
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, LoginRequestData& data)
{
    packet >> data.username >> data.password;
    return packet;
}

// LoginResponseData
inline sf::Packet& operator<<(sf::Packet& packet, const LoginResponseData& data)
{
    packet << data.success
        << data.playerId
        << data.username
        << data.score
        << data.message;
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, LoginResponseData& data)
{
    packet >> data.success
        >> data.playerId
        >> data.username
        >> data.score
        >> data.message;
    return packet;
}

// RegisterRequestData
inline sf::Packet& operator<<(sf::Packet& packet, const RegisterRequestData& data)
{
    packet << data.username << data.password;
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, RegisterRequestData& data)
{
    packet >> data.username >> data.password;
    return packet;
}

// RegisterResponseData
inline sf::Packet& operator<<(sf::Packet& packet, const RegisterResponseData& data)
{
    packet << data.success << data.message;
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, RegisterResponseData& data)
{
    packet >> data.success >> data.message;
    return packet;
}

// CreateRoomRequestData
inline sf::Packet& operator<<(sf::Packet& packet, const CreateRoomRequestData& data)
{
    packet << data.roomId << data.username << data.gamePort;
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, CreateRoomRequestData& data)
{
    packet >> data.roomId >> data.username >> data.gamePort;
    return packet;
}

// CreateRoomResponseData
inline sf::Packet& operator<<(sf::Packet& packet, const CreateRoomResponseData& data)
{
    packet << data.success << data.roomId << data.message;
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, CreateRoomResponseData& data)
{
    packet >> data.success >> data.roomId >> data.message;
    return packet;
}

// JoinRoomRequestData
inline sf::Packet& operator<<(sf::Packet& packet, const JoinRoomRequestData& data)
{
    packet << data.roomId << data.username << data.gamePort;
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, JoinRoomRequestData& data)
{
    packet >> data.roomId >> data.username >> data.gamePort;
    return packet;
}

// JoinRoomResponseData
inline sf::Packet& operator<<(sf::Packet& packet, const JoinRoomResponseData& data)
{
    packet << data.success << data.roomId << data.message;
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, JoinRoomResponseData& data)
{
    packet >> data.success >> data.roomId >> data.message;
    return packet;
}

// ErrorMessageData
inline sf::Packet& operator<<(sf::Packet& packet, const ErrorMessageData& data)
{
    packet << data.message;
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, ErrorMessageData& data)
{
    packet >> data.message;
    return packet;
}

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


sf::Packet& operator<<(sf::Packet& packet, const RoomStatusUpdateData& data);
sf::Packet& operator>>(sf::Packet& packet, RoomStatusUpdateData& data);

sf::Packet& operator<<(sf::Packet& packet, const StartGameData& data);
sf::Packet& operator>>(sf::Packet& packet, StartGameData& data);

// Launcher / verificacion de mapa
struct MapCheckData
{
    std::string version;
};

struct MapStatusData
{
    bool upToDate = false;
};

struct MapResponseData
{
    std::string version;
    std::string mapContent;
};

inline sf::Packet& operator<<(sf::Packet& packet, const MapCheckData& data)
{
    return packet << data.version;
}
inline sf::Packet& operator>>(sf::Packet& packet, MapCheckData& data)
{
    return packet >> data.version;
}
inline sf::Packet& operator<<(sf::Packet& packet, const MapStatusData& data)
{
    return packet << data.upToDate;
}
inline sf::Packet& operator>>(sf::Packet& packet, MapStatusData& data)
{
    return packet >> data.upToDate;
}
inline sf::Packet& operator<<(sf::Packet& packet, const MapResponseData& data)
{
    return packet << data.version << data.mapContent;
}
inline sf::Packet& operator>>(sf::Packet& packet, MapResponseData& data)
{
    return packet >> data.version >> data.mapContent;
}

// RankingUpdateData
inline sf::Packet& operator<<(sf::Packet& packet, const RankingUpdateData& data)
{
    packet << data.roomId;
    packet << static_cast<int>(data.placementOrder.size());
    for (int playerId : data.placementOrder)
    {
        packet << playerId;
    }
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, RankingUpdateData& data)
{
    int size = 0;
    packet >> data.roomId >> size;
    data.placementOrder.clear();
    for (int i = 0; i < size; ++i)
    {
        int playerId;
        packet >> playerId;
        data.placementOrder.push_back(playerId);
    }
    return packet;
}
