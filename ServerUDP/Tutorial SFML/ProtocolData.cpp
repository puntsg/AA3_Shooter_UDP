#include "ProtocolData.h"


sf::Packet& operator<<(sf::Packet& packet, const SessionStartData& data)
{
    packet << data.roomId << data.playerCount;
    for (const LobbyPlayerInfo& p : data.players)
        packet << p;
    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, SessionStartData& data)
{
    packet >> data.roomId >> data.playerCount;
    data.players.clear();

    for (int i = 0; i < data.playerCount; i++)
    {
        LobbyPlayerInfo p;
        packet >> p;
        data.players.push_back(p);
    }
    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const SessionStartResponseData& data)
{
    packet << data.success << data.roomId << data.message;
    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, SessionStartResponseData& data)
{
    packet >> data.success >> data.roomId >> data.message;
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
    for (const Result& result : data.results)
        packet << result;

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, GameResultData& data)
{
    int count = 0;
    packet >> data.roomId >> count;
    data.results.resize(count);

    for (int i = 0; i < count; i++)
        packet >> data.results[i];

    return packet;
}
