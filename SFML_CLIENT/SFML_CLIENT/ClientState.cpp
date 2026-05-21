#include "ClientState.h"

void ClientState::ResetRoomState()
{
    currentRoomId.clear();
    isHost = false;
    isWaitingInRoom = false;
    hasGameStarted = false;
    roomPlayers.clear();
}

void ClientState::ResetAuthState()
{
    playerId = -1;
    nickname.clear();
    authMessage.clear();
    authMessageIsError = false;
}

void ClientState::ResetAll()
{
    ResetAuthState();
    ResetRoomState();
    ranking.clear();
}
