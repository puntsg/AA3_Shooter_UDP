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
