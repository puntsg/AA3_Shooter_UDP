#include "ClientState.h"

void ClientState::ResetRankingState()
{
    ranking.clear();
    rankingLoading = false;
    rankingReceived = false;
    rankingMessageIsError = false;
    rankingMessage.clear();
}
