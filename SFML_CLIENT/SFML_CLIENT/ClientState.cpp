#include "ClientState.h"

void ClientState::ResetRoomState()
{
    currentRoomId.clear();
    isHost = false;
    isWaitingInRoom = false;
    isSearchingMatch = false;
    searchingRanked = false;
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

void ClientState::ResetRankingState()
{
    ranking.clear();
    rankingLoading = false;
    rankingReceived = false;
    rankingMessageIsError = false;
    rankingMessage.clear();
}

void ClientState::ResetAll()
{
    ResetAuthState();
    ResetRoomState();
    ResetRankingState();
}
