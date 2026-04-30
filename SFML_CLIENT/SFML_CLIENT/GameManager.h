#pragma once
#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "Player.h"
#include "PacketTypes.h"

class GameManager
{
private:
    std::vector<Player> players;
    int currentTurnIndex = 0;
    float turnTimer = 0.0f;

    std::vector<int> victoryOrder;
    bool isGameOver = false;
    sf::Font font;
    int localPlayerID = 1;

    /// gx Columna 
    /// gy Fila 
    void BroadcastMove(int gx, int gy, int playerID);

    void HandlePeerDisconnection(sf::TcpSocket* socket);

    int GetPlayerIndexBySocket(sf::TcpSocket* socket) const;

public:
    
    std::vector<std::vector<short>> grid;

    GameManager();

    void InitGame(const std::vector<Player>& connectedPlayers, int localID);

    void Update(float dt);
   
    void TryPlacePieceScreen(float mouseX, float mouseY);
 
    bool TryPlacePieceGrid(int gx, int gy, int playerIndex);

    void ReceiveNetworkMoves();
  
    bool CheckWin(int gx, int gy, int playerID);
   
    void AdvanceTurn();

    void SyncNextTurn(int nextPlayerID);
    void BroadcastNextTurn(int nextPlayerID);

    void CheckGameOver();
   
    void DrawGrid(sf::RenderWindow& window);
   
    void DrawHUD(sf::RenderWindow& window);

    // Limpia todo el estado de partida
    void Reset();
};
