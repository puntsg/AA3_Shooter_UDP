#pragma once

#include <string>
#include <vector>

struct Room
{
    std::string roomId = "";
    std::vector<int> playerIds;
    bool inGame = false;
    int maxPlayers = 4;
};