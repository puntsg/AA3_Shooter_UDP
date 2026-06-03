#include "ProtocolData.h"

sf::Packet& operator<<(sf::Packet& packet, const RankingResponseData& data)
{
    packet << data.success << data.message << static_cast<int>(data.entries.size());
    for (const RankingData& entry : data.entries)
        packet << entry;
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
    packet << data.roomId << static_cast<int>(data.results.size());
    for (const Result& r : data.results)
        packet << r;

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, GameResultData& data)
{
    int count = 0;
    packet >> data.roomId >> count;
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
        << data.playerCount
        << data.gameServerIp
        << data.gameServerUdpPort;

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
        >> data.gameServerIp
        >> data.gameServerUdpPort
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
