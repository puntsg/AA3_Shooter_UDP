#pragma once

#include <string>
#include <vector>
#include "ProtocolData.h"

struct ClientState
{
    // Identidad del jugador
    int playerId = -1;
    std::string nickname = "";
    std::string savedPassword = "";
    std::string authMessage = "";
    bool authMessageIsError = false;
    unsigned short myGamePort = 0;

    // Estado de la sala
    std::string currentRoomId = "";
    bool isHost = false;
    bool isWaitingInRoom = false;
    bool isSearchingMatch = false;
    bool searchingRanked = false;

    // Estado de partida
    bool hasGameStarted = false;

    // Ranking pendiente al terminar partida P2P
    bool hasPendingResult = false;
    GameResultData pendingGameResult;

    // Jugadores actuales de la sala
    std::vector<LobbyPlayerInfo> roomPlayers;
    std::vector<RankingData> ranking;
    bool rankingLoading = false;
    bool rankingReceived = false;
    bool rankingMessageIsError = false;
    std::string rankingMessage = "";

    void ResetRoomState();

    void ResetAuthState();

    void ResetRankingState();

    void ResetAll();

    inline bool IsLoggedIn() const {return playerId != -1 && !nickname.empty();}

    inline bool IsInRoom() const { return !currentRoomId.empty();}
};
