#include "GameManager.h"
#include "NetworkManager.h"
#include <string>
#include <algorithm>
#include "Constants.h"

GameManager::GameManager()
{
    grid.assign(Config::Game::GRID_COLUMNS, std::vector<short>(Config::Game::GRID_ROWS, 0));

    if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf"))
    {
        std::cerr << "Warning: Arial font not found." << std::endl;
    }
}

void GameManager::InitGame(const std::vector<Player>& connectedPlayers, int localID)
{

    // Reset board
    grid.assign(Config::Game::GRID_COLUMNS, std::vector<short>(Config::Game::GRID_ROWS, 0));

    // Save players 
    players = connectedPlayers;
    localPlayerID = localID;

    // Assign colors
    for (size_t i = 0; i < players.size() && i < 4; ++i)
    {
        players[i].color = Config::Game::PLAYER_COLORS[i];
    }

    currentTurnIndex = 0;
    turnTimer = Config::Game::MAX_TURN_TIME;
    victoryOrder.clear();
    isGameOver = false;

    std::cout << "--- Game Started (P2P) ---" << std::endl;
    std::cout << "My ID: " << localPlayerID << std::endl;
    for (const auto& p : players)
    {
        std::cout << "Player: " << p.nickName << " (ID: " << p.id << ")" << std::endl;
    }
}

void GameManager::Update(float dt)
{
    if (isGameOver) return;
    if (players.empty()) return;

    // Read packets
    ReceiveNetworkMoves();

    if (isGameOver || players.empty()) return;

    // Simple turn timer logic
    turnTimer -= dt;
    if (turnTimer <= 0.0f)
    {
        std::cout << "Time out for " << players[currentTurnIndex].nickName << std::endl;
        AdvanceTurn();
    }
}

void GameManager::ReceiveNetworkMoves()
{
    auto& connections = NM.GetConnections();
    for (auto& sock : connections)
    {
        if (!sock) continue;

        sf::Packet packet;
        sf::Socket::Status status = sock->receive(packet);

        if (status == sf::Socket::Status::Done)
        {
            int packetType = -1;
            packet >> packetType;

            if (packetType == PacketType::PIECEADDED)
            {
                int senderID = 0;
                int gx = 0, gy = 0;
                packet >> senderID >> gx >> gy;

                // Find who sent it
                int playerIdx = -1;
                for (int i = 0; i < (int)players.size(); i++)
                {
                    if (players[i].id == senderID) {
                        playerIdx = i;
                        break;
                    }
                }

                if (playerIdx != -1)
                {
                    std::cout << "Move from " << players[playerIdx].nickName << " at " << gx << "," << gy << std::endl;
                    TryPlacePieceGrid(gx, gy, playerIdx);
                }
            }
            else if (packetType == PacketType::PLAYER_DISCONNECTED)
            {
                int disconnectedID = 0;
                packet >> disconnectedID;
                
                for (int i = 0; i < (int)players.size(); i++)
                {
                    if (players[i].id == disconnectedID && !players[i].isSpectator)
                    {
                        players[i].isSpectator = true;
                        std::cout << players[i].nickName << " disconnected (notified by peer)" << std::endl;
                        if (i == currentTurnIndex) AdvanceTurn();
                        break;
                    }
                }
                CheckGameOver();
            }
            else if (packetType == PacketType::NEXT_TURN)
            {
                int nextID = 0;
                packet >> nextID;
                SyncNextTurn(nextID);
            }
        }
        else if (status == sf::Socket::Status::Disconnected || status == sf::Socket::Status::Error)
        {
            HandlePeerDisconnection(sock.get());
        }
    }
}

void GameManager::HandlePeerDisconnection(sf::TcpSocket* socket)
{
    int playerIdx = GetPlayerIndexBySocket(socket);
    if (playerIdx == -1) return;

    Player& p = players[playerIdx];
    if (p.isSpectator) return;

    std::cout << p.nickName << " disconnected. Kicking them out." << std::endl;
    p.isSpectator = true;

    sf::Packet notify;
    notify << (int)PacketType::PLAYER_DISCONNECTED << p.id;
    NM.SendToAllConnections(notify);

    if (playerIdx == currentTurnIndex) AdvanceTurn();
    CheckGameOver();
}

int GameManager::GetPlayerIndexBySocket(sf::TcpSocket* socket) const
{
    auto& connections = NM.GetConnections();
    int connIdx = 0;
    for (int i = 0; i < (int)players.size(); i++)
    {
        if (players[i].id == localPlayerID) continue;
        if (connIdx < (int)connections.size() && connections[connIdx].get() == socket) return i;
        connIdx++;
    }
    return -1;
}

void GameManager::BroadcastMove(int gx, int gy, int playerID)
{
    sf::Packet packet;
    packet << (int)PacketType::PIECEADDED << playerID << gx << gy;
    NM.SendToAllConnections(packet);
}

void GameManager::BroadcastNextTurn(int nextPlayerID)
{
    sf::Packet packet;
    packet << (int)PacketType::NEXT_TURN << nextPlayerID;
    NM.SendToAllConnections(packet);
}

void GameManager::SyncNextTurn(int nextPlayerID)
{
    for (int i = 0; i < (int)players.size(); i++)
    {
        if (players[i].id == nextPlayerID)
        {
            if (currentTurnIndex != i)
            {
                currentTurnIndex = i;
                turnTimer = Config::Game::MAX_TURN_TIME;
                std::cout << "Network Sync: Turn passed to " << players[i].nickName << std::endl;
            }
            break;
        }
    }
}

void GameManager::TryPlacePieceScreen(float mouseX, float mouseY)
{
    if (isGameOver || players.empty()) return;
    if (players[currentTurnIndex].id != localPlayerID) return;

    float offsetX = (Config::Window::WIDTH - (Config::Game::GRID_COLUMNS * Config::Game::CELL_SIZE)) / 2.f;
    float offsetY = (Config::Window::HEIGHT - (Config::Game::GRID_ROWS * Config::Game::CELL_SIZE)) / 2.f;

    // Ignore clicks outside grid
    if (mouseX < offsetX || mouseX > offsetX + (Config::Game::GRID_COLUMNS * Config::Game::CELL_SIZE) ||
        mouseY < offsetY || mouseY > offsetY + (Config::Game::GRID_ROWS * Config::Game::CELL_SIZE)) return;

    int gx = (int)((mouseX - offsetX) / Config::Game::CELL_SIZE);
    int gy = (int)((mouseY - offsetY) / Config::Game::CELL_SIZE);

    TryPlacePieceGrid(gx, gy, currentTurnIndex);
}

bool GameManager::TryPlacePieceGrid(int gx, int gy, int playerIndex)
{
    if (gx < 0 || gx >= Config::Game::GRID_COLUMNS || gy < 0 || gy >= Config::Game::GRID_ROWS) return false;
    if (grid[gx][gy] != 0) return false;

    int playerID = players[playerIndex].id;
    grid[gx][gy] = (short)playerID;

    std::cout << ">>> " << players[playerIndex].nickName << " placed at " << gx << "," << gy << std::endl;

    // Send to plaayers if it's my turn
    if (playerID == localPlayerID) BroadcastMove(gx, gy, playerID);

    if (CheckWin(gx, gy, playerID))
    {
        players[playerIndex].isSpectator = true;
        victoryOrder.push_back(playerID);
        std::cout << "!!! " << players[playerIndex].nickName << " WON!" << std::endl;
        CheckGameOver();
    }

    if (!isGameOver) AdvanceTurn();
    return true;
}

bool GameManager::CheckWin(int gx, int gy, int playerID)
{
    const int dirs[4][2][2] = {
        {{-1, 0}, {1, 0}},  // Horiz
        {{0, -1}, {0, 1}},  // Vert
        {{-1,-1}, {1, 1}},  // Diag 
        {{-1, 1}, {1,-1}}   // Diag
    };

    for (int d = 0; d < 4; d++) {
        int count = 1;
        for (int side = 0; side < 2; side++) {
            int k = 1;
            while (true) {
                int nx = gx + dirs[d][side][0] * k;
                int ny = gy + dirs[d][side][1] * k;
                if (nx < 0 || nx >= Config::Game::GRID_COLUMNS || ny < 0 || ny >= Config::Game::GRID_ROWS) break;
                if (grid[nx][ny] != playerID) break;
                count++;
                k++;
            }
        }
        if (count >= 3) return true;
    }
    return false;
}

void GameManager::AdvanceTurn()
{
    // Check if board is full
    bool boardFull = true;
    for (int x = 0; x < Config::Game::GRID_COLUMNS; x++) {
        for (int y = 0; y < Config::Game::GRID_ROWS; y++) {
            if (grid[x][y] == 0) boardFull = false;
        }
    }

    if (boardFull) {
        std::cout << "Draw! Board is full." << std::endl;
        CheckGameOver();
        return;
    }

    // Move to next player that is not a spectator
    int total = (int)players.size();
    for (int i = 0; i < total; i++) {
        currentTurnIndex = (currentTurnIndex + 1) % total;
        if (!players[currentTurnIndex].isSpectator) {
            turnTimer = Config::Game::MAX_TURN_TIME;
            std::cout << "--- Turn: " << players[currentTurnIndex].nickName << " ---" << std::endl;

           
            BroadcastNextTurn(players[currentTurnIndex].id);
            return;
        }
    }
    CheckGameOver();
}

void GameManager::CheckGameOver()
{
    int spectators = 0;
    for (const auto& p : players) if (p.isSpectator) spectators++;

    bool boardFull = true;
    for (int x = 0; x < Config::Game::GRID_COLUMNS; x++)
        for (int y = 0; y < Config::Game::GRID_ROWS; y++)
            if (grid[x][y] == 0) boardFull = false;

    
    if (spectators >= (int)players.size() - 1 || boardFull)
    {
        std::cout << "=== GAME OVER ===" << std::endl;
        isGameOver = true;
        
        GameResultData resultData;
        for (int i = 0; i < players.size(); i++) {
            Result r;
            r.username = players[i].nickName;
            r.scoredPoints = Config::Game::LOSE_GAME;
            for (int j = 0; j < victoryOrder.size(); j++)
            {
                if (victoryOrder[j] == players[i].id) {
                    r.scoredPoints = Config::Game::WIN_GAME;
                    break;
                }
            }
            resultData.results.push_back(r);
        }

        NM.ClearConnections();

        // Reconnect to Bootstrap server
        if (NM.ConnectToServer()) 
        {
            // Login save
            std::string savedUser = NM.GetClientState().nickname;
            std::string savedPass = NM.GetClientState().savedPassword;
            NM.SendLoginRequest(savedUser, savedPass);
            
           
            sf::Packet packet;
            packet << static_cast<short>(PacketType::ENDGAME);
            packet << resultData;
            NM.SendToServer(packet);
            
            std::cout << "[CLIENT] Reconectado y ENDGAME enviado." << std::endl;
        }
        else
        {
            std::cerr << "[CLIENT] Fallo al conectar despues de la partida." << std::endl;
        }

        NM.GetClientState().hasPendingResult = false;
        SM.SetNextScene("LobbyScene");
    }
}

void GameManager::DrawGrid(sf::RenderWindow& window)
{
    float offsetX = (800.f - (Config::Game::GRID_COLUMNS * Config::Game::CELL_SIZE)) / 2.f;
    float offsetY = (600.f - (Config::Game::GRID_ROWS * Config::Game::CELL_SIZE)) / 2.f;

    for (int x = 0; x < Config::Game::GRID_COLUMNS; x++) {
        for (int y = 0; y < Config::Game::GRID_ROWS; y++) {
            sf::RectangleShape cell({ (float)Config::Game::CELL_SIZE - 2.f, (float)Config::Game::CELL_SIZE - 2.f });
            cell.setPosition({ offsetX + (x * Config::Game::CELL_SIZE), offsetY + (y * Config::Game::CELL_SIZE) });

            sf::Color color = sf::Color(50, 50, 50);
            if (grid[x][y] != 0) {
                for (const auto& p : players) {
                    if (p.id == grid[x][y]) {
                        color = p.color;
                        break;
                    }
                }
            }
            cell.setFillColor(color);
            window.draw(cell);
        }
    }
}

void GameManager::DrawHUD(sf::RenderWindow& window)
{
    if (players.empty()) return;

    sf::Text text(font);
    text.setCharacterSize(20);
    text.setPosition({ 20.f, 20.f });

    std::string str = "Turn: " + players[currentTurnIndex].nickName + "\n";
    str += "Time: " + std::to_string((int)turnTimer) + "s";
    if (players[currentTurnIndex].id == localPlayerID) str += " (YOUR TURN)";

    text.setString(str);
    text.setFillColor(players[currentTurnIndex].color);
    window.draw(text);

    float yPos = 20.f;
    for (const auto& p : players) {
        sf::Text score(font);
        score.setCharacterSize(16);
        score.setPosition({ 600.f, yPos });

        std::string scoreStr = p.nickName + ": " + std::to_string(p.scoreRanking);
        if (p.isSpectator) scoreStr += " (ESP)";

        score.setString(scoreStr);
        score.setFillColor(p.color);
        window.draw(score);

        yPos += 25.f;
    }
}

void GameManager::Reset()
{
    players.clear();
    victoryOrder.clear();
    grid.assign(Config::Game::GRID_COLUMNS, std::vector<short>(Config::Game::GRID_ROWS, 0));
    isGameOver = false;
    currentTurnIndex = 0;
    turnTimer = Config::Game::MAX_TURN_TIME;
    localPlayerID = -1;
    std::cout << "[CLIENT] GameManager reseteado." << std::endl;
}
