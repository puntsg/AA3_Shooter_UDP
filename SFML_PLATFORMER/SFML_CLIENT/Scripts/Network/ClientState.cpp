#include "ClientState.h"

void ClientState::ResetRoomState()
{
    currentRoomId.clear();
    isHost = false;
    isWaitingInRoom = false;
    hasGameStarted = false;
    roomPlayers.clear();
}

void ClientState::ResetAll()
{
    playerId = -1;
    nickname.clear();
    ResetRoomState();
    ranking.clear();
}
