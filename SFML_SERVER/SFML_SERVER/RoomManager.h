#pragma once

#include <vector>
#include <string>
#include "Room.h"

#define MAX_PLAYERS 2

class RoomManager
{
public:
    bool CreateRoom(const std::string& roomId, int creatorPlayerId);
    bool JoinRoom(const std::string& roomId, int playerId);

    void RemovePlayerFromRoom(int playerId);
    void DeleteRoom(const std::string& roomId);

    Room* GetRoom(const std::string& roomId);
    Room* GetRoomByPlayerId(int playerId);

    bool RoomExists(const std::string& roomId) const;

    void PrintRooms() const;

private:
    std::vector<Room> m_rooms;
};
