#include "ProtocolData.h"

sf::Packet& operator<<(sf::Packet& packet, const RankingResponseData& data)
{
    packet << data.success << data.message << static_cast<int>(data.entries.size());
    for (int i = 0; i < (int)data.entries.size(); i++)
        packet << data.entries[i];
    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, RankingResponseData& data)
{
    int count = 0;
    packet >> data.success >> data.message >> count;
    data.entries.resize(count);
    for (int i = 0; i < count; i++)
        packet >> data.entries[i];
    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const Result& data)
{
    packet << data.username << data.scoredPoints;
    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, Result& data)
{
    packet >> data.username >> data.scoredPoints;
    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const GameResultData& data)
{
    packet << static_cast<int>(data.results.size());
    for (const Result& r : data.results)
        packet << r;

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, GameResultData& data)
{
    int count = 0;
    packet >> count;
    data.results.resize(count);
    for (int i = 0; i < count; ++i)
        packet >> data.results[i];

    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const RoomStatusUpdateData& data)
{
    packet << data.roomId
        << data.currentPlayers
        << data.maxPlayers;

    packet << static_cast<int>(data.players.size());
    for (const LobbyPlayerInfo& player : data.players)
    {
        packet << player;
    }

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, RoomStatusUpdateData& data)
{
    int playerCount = 0;

    packet >> data.roomId
        >> data.currentPlayers
        >> data.maxPlayers
        >> playerCount;

    data.players.clear();
    data.players.resize(playerCount);

    for (int i = 0; i < playerCount; ++i)
    {
        packet >> data.players[i];
    }

    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const StartGameData& data)
{
    packet << data.roomId
        << data.playerCount;

    packet << static_cast<int>(data.players.size());
    for (const LobbyPlayerInfo& player : data.players)
    {
        packet << player;
    }

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, StartGameData& data)
{
    int vectorSize = 0;

    packet >> data.roomId
        >> data.playerCount
        >> vectorSize;

    data.players.clear();
    data.players.resize(vectorSize);

    for (int i = 0; i < vectorSize; ++i)
    {
        packet >> data.players[i];
    }

    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const SessionStartData& data)
{
    packet << data.roomId
        << data.playerCount;

    // Enviamos los players tal cual para que el Game Server sepa a quien esperar
    for (const LobbyPlayerInfo& player : data.players)
    {
        packet << player;
    }

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, SessionStartData& data)
{
    packet >> data.roomId
        >> data.playerCount;

    data.players.clear();

    // playerCount marca cuantos LobbyPlayerInfo vienen despues
    for (int i = 0; i < data.playerCount; ++i)
    {
        LobbyPlayerInfo player;
        packet >> player;
        data.players.push_back(player);
    }

    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const SessionStartResponseData& data)
{
    packet << data.success
        << data.roomId
        << data.message;

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, SessionStartResponseData& data)
{
    packet >> data.success
        >> data.roomId
        >> data.message;

    return packet;
}
