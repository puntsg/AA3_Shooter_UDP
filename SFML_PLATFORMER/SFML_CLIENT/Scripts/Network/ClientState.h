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
    unsigned short myGamePort = 0;

    // Estado de la sala
    std::string currentRoomId = "";
    bool isHost = false;
    bool isWaitingInRoom = false;

    // Estado de partida
    bool hasGameStarted = false;

    // Ranking pendiente al terminar partida P2P
    bool hasPendingResult = false;
    GameResultData pendingGameResult;

    // Jugadores actuales de la sala
    std::vector<LobbyPlayerInfo> roomPlayers;
    std::vector<RankingData> ranking;

    void ResetRoomState();

    void ResetAll();

    inline bool IsLoggedIn() const {return playerId != -1 && !nickname.empty();}

    inline bool IsInRoom() const { return !currentRoomId.empty();}
};