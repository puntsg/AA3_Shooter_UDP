#pragma once

#include <SFML/Network.hpp>
#include <string>

struct ConnectedClient
{
    int playerId;
    std::string username;
    sf::TcpSocket* socket;
    sf::IpAddress ip;
    unsigned short gamePort;
    std::string currentRoomId;

    ConnectedClient()
        : playerId(-1)
        , username("")
        , socket(nullptr)
        , ip(sf::IpAddress::Any)
        , gamePort(0)
        , currentRoomId("")
    {
    }
};