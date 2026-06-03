#pragma once

#include <SFML/Network.hpp>
#include <string>

struct ConnectedClient
{
    int playerId;
    std::string username;
    sf::TcpSocket* socket;
    sf::IpAddress ip;
    std::string currentRoomId;

    ConnectedClient()
        : playerId(-1)
        , username("")
        , socket(nullptr)
        , ip(sf::IpAddress::Any)
        , currentRoomId("")
    {
    }
};
