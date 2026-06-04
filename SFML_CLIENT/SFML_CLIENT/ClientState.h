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

    // Estado de la sala
    std::string currentRoomId = "";
    bool isHost = false;
    bool isWaitingInRoom = false;
    bool isSearchingMatch = false;
    bool searchingRanked = false;

    // Estado de partida
    bool hasGameStarted = false;
    std::string gameServerIp = "";
    unsigned short gameServerUdpPort = 0;

    // Jugadores actuales de la sala
    std::vector<LobbyPlayerInfo> roomPlayers;
    std::vector<RankingData> ranking;
    bool rankingLoading = false;
    bool rankingReceived = false;
    bool rankingMessageIsError = false;
    std::string rankingMessage = "";

    // Estado UDP del juego 
    std::vector<TransformData>  incomingTransforms;  // ultima pos recibida por jugador
    bool             hasShootReplicate = false;
    ShootReplicateData lastShootReplicate;
    bool             hasPlayerHit = false;
    PlayerHitData    lastPlayerHit;
    bool             hasTaunt = false;
    int              tauntPlayerId = -1;
    bool             hasEndgame = false;
    EndgameData      endgameData;

    void ResetRankingState();

    inline bool IsLoggedIn() const {return playerId != -1 && !nickname.empty();}
};
