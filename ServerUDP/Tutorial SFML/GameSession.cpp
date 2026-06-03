#include "GameSession.h"
#include <iostream>
#include <cmath>
#include <fstream>
#include <algorithm>

static constexpr float TILE_SIZE = 32.f;
static constexpr float PROJECTILE_STEP = 4.f;
static constexpr float PLAYER_HIT_HALF_WIDTH = 16.f;
static constexpr float PLAYER_HIT_HALF_HEIGHT = 24.f;
static constexpr float BULLET_HIT_RADIUS = 5.f;
static constexpr float SHOT_SPAWN_OFFSET = 16.f;
static constexpr float MAX_CLIENT_SHOT_OFFSET = 48.f;
static constexpr const char* RANKING_SERVER_IP = "127.0.0.1";
static constexpr unsigned short RANKING_SERVER_PORT = 55001;
static constexpr int WIN_POINTS = 20;
static constexpr int LOSE_POINTS = -5;

static bool IsRankedRoomId(const std::string& roomId)
{
    const std::string rankedPrefix = "match_ranked_";
    return roomId.compare(0, rankedPrefix.size(), rankedPrefix) == 0;
}

GameSession::GameSession(const std::string& roomId, const LobbyPlayerInfo& p1Info, const LobbyPlayerInfo& p2Info, sf::UdpSocket& socket, std::mutex& socketMutex)
    : roomId(roomId)
    , socket(socket)
    , socketMutex(socketMutex)
    , finished(false)
    , bothReady(false)
{
    playerIds[0] = p1Info.playerId;
    playerIds[1] = p2Info.playerId;

    // Guardamos nombres para poder actualizar ranking al acabar.
    playerNames[0] = p1Info.username;
    playerNames[1] = p2Info.username;

    // se llena con el hello udp
    states[0].ip = sf::IpAddress::Any;
    states[0].port = 0;
    states[0].position = sf::Vector2f(P1_START_X, START_Y);

    states[1].ip = sf::IpAddress::Any;
    states[1].port = 0;
    states[1].position = sf::Vector2f(P2_START_X, START_Y);

    LoadCollisionMap();

    std::cout << "Room " << roomId << " esperando UDP. P1: " << p1Info.playerId
              << " P2: " << p2Info.playerId << std::endl;
}

void GameSession::ProcessMovePacket(int playerId, sf::Packet& packet)
{
    TransformData moveData;
    packet >> moveData;

    PlayerState& state = GetState(playerId);
    float timeSinceLast = state.lastPacketClock.restart().asSeconds();

    if (state.lastValidPacketId != 0 && moveData.packetId <= state.lastValidPacketId)
        return;

    sf::Vector2f newPos(moveData.x, moveData.y);
    
    float dx = std::abs(newPos.x - state.position.x);
    float dy = std::abs(newPos.y - state.position.y);
    bool bigMove = dx > CHEAT_THRESHOLD || dy > CHEAT_THRESHOLD;

    if (bigMove)
    {
        state.cheatingStrikes++;
        std::cout << "Movement warning in " << roomId
            << " p" << playerId
            << " dx: " << dx
            << " dy: " << dy
            << " alerts: " << state.cheatingStrikes
            << std::endl;

        if (state.cheatingStrikes > MAX_STRIKES)
            state.cheatingStrikes = MAX_STRIKES;
    }
    else if (state.cheatingStrikes > 0)
    {
        state.cheatingStrikes--;
    }

    if (!bigMove && timeSinceLast > 0.f)
    {
        state.velocity.x = (newPos.x - state.position.x) / timeSinceLast;
        state.velocity.y = (newPos.y - state.position.y) / timeSinceLast;
    }
    else
    {
        state.velocity = sf::Vector2f(0.f, 0.f);
    }

    state.position = newPos;
    state.flipped = moveData.flipped;
    state.spriteStartX = moveData.spriteStartX;
    state.spriteStartY = moveData.spriteStartY;
    state.spriteEndX = moveData.spriteEndX;
    state.spriteEndY = moveData.spriteEndY;
    state.lastValidPacketId = moveData.packetId;
}

void GameSession::ProcessShotPacket(int playerId, sf::Packet& packet)
{
    PlayerState& shooter = GetState(playerId);
    shooter.lastPacketClock.restart();

    ShootReplicateData shotData;
    packet >> shotData;
    bool validShotPacket = static_cast<bool>(packet);

    shotData.flipped = shooter.flipped;
    float dir = shotData.flipped ? 1.f : -1.f;
    sf::Vector2f authoritativePos = shooter.position + sf::Vector2f(dir * SHOT_SPAWN_OFFSET, 0.f);

    if (!validShotPacket)
    {
        shotData.position = authoritativePos;
    }
    else
    {
        float dx = shotData.position.x - authoritativePos.x;
        float dy = shotData.position.y - authoritativePos.y;
        float offset = std::sqrt(dx * dx + dy * dy);
        if (offset > MAX_CLIENT_SHOT_OFFSET)
        {
            std::cout << "[UDP-SHOT-WARN] Room " << roomId
                << " shooter: " << playerId
                << " ignored client shot offset: " << offset
                << std::endl;
            shotData.position = authoritativePos;
        }
    }

    std::cout << "[UDP-SHOT] Room " << roomId
        << " shooter: " << playerId
        << " pos: " << shotData.position.x << ", " << shotData.position.y
        << std::endl;

    BulletState bullet;
    bullet.position = shotData.position;
    bullet.velocity = sf::Vector2f(dir * BULLET_SPEED, 0.f);
    bullet.flipped = shotData.flipped;
    bullet.ownerID = playerId;
    bullet.traveled = 0.f;
    bullet.active = true;

    if (IsWallAt(bullet.position))
    {
        std::cout << "[UDP-MISS] Room " << roomId
            << " shooter: " << playerId
            << " reason: wall_spawn"
            << std::endl;
        return;
    }

    {
        std::lock_guard<std::mutex> lock(bulletsMutex);
        bullets.push_back(bullet);
    }

    ShootReplicateData replicateData;
    replicateData.position = shotData.position;
    replicateData.flipped = shotData.flipped;

    sf::Packet replicatePacket;
    replicatePacket << PacketType::SHOOT_REPLICATE << replicateData;
    SendToOther(playerId, replicatePacket);
}

void GameSession::ProcessTauntPacket(int playerId)
{
    GetState(playerId).lastPacketClock.restart();

    sf::Packet tauntPacket;
    tauntPacket << PacketType::PLAYER_TAUNT << playerId;

    std::lock_guard<std::mutex> lock(socketMutex);
    for (int i = 0; i < 2; i++)
    {
        if (states[i].ready && !states[i].disconnected)
            socket.send(tauntPacket, states[i].ip, states[i].port);
    }
}

void GameSession::ProcessReadyPacket(int playerId)
{
    PlayerState& state = GetState(playerId);
    state.ready = true;

    if (states[0].ready && states[1].ready)
    {
        bothReady = true;
        std::cout << "Both ready in " << roomId << std::endl;
    }
}

bool GameSession::RegisterPlayerEndpoint(int playerId, const sf::IpAddress& ip, unsigned short port)
{
    int index = -1;
    if (playerIds[0] == playerId)
        index = 0;
    else if (playerIds[1] == playerId)
        index = 1;

    if (index == -1)
        return false;

    states[index].ip = ip;
    states[index].port = port;
    states[index].ready = true;
    states[index].disconnected = false;
    states[index].lastPacketClock.restart();

    std::cout << "UDP ready p" << playerId << " -> "
        << ip.toString() << ":" << port << std::endl;

    if (states[0].ready && states[1].ready)
    {
        bothReady = true;
        std::cout << "Both ready in " << roomId << std::endl;
    }

    return true;
}

void GameSession::DisconnectPlayer(int playerId)
{
    if (finished)
        return;

    int winnerId = GetOtherPlayerId(playerId);
    if (winnerId == -1)
        return;

    GetState(playerId).disconnected = true;
    SendPlayerDisconnected(playerId);
    FinishGame(winnerId, false);
}

void GameSession::Update(float dt)
{
    if (finished)
        return;

    if (!bothReady)
    {
        CheckHelloTimeout();
        return;
    }

    CheckDisconnects();
    if (finished)
        return;

    PredictPositions(dt);
    UpdateBullets(dt);
    if (finished)
        return;

    
    //60fps = 0.016
    if (broadcastClock.getElapsedTime().asSeconds() >= 0.016f)
    {
        BroadcastGameState();
        broadcastClock.restart();
    }
}

bool GameSession::IsFinished() const
{
    return finished;
}

std::string GameSession::GetRoomId() const
{
    return roomId;
}

bool GameSession::BelongsToSession(const sf::IpAddress& ip, unsigned short port) const
{
    return (states[0].ready && states[0].ip == ip && states[0].port == port) ||
           (states[1].ready && states[1].ip == ip && states[1].port == port);
}

int GameSession::GetPlayerIdByAddress(const sf::IpAddress& ip, unsigned short port) const
{
    if (states[0].ip == ip && states[0].port == port)
        return playerIds[0];
    if (states[1].ip == ip && states[1].port == port)
        return playerIds[1];
    return -1;
}

void GameSession::BroadcastGameState()
{
    std::lock_guard<std::mutex> lock(socketMutex);
    for (int i = 0; i < 2; i++)
    {
        TransformData tData;
        tData.packetId = states[i].lastValidPacketId;
        tData.dbId = playerIds[i];
        tData.localPlayerId = i;
        tData.x = states[i].position.x;
        tData.y = states[i].position.y;
        tData.flipped = states[i].flipped;
        tData.spriteStartX = states[i].spriteStartX;
        tData.spriteStartY = states[i].spriteStartY;
        tData.spriteEndX = states[i].spriteEndX;
        tData.spriteEndY = states[i].spriteEndY;

        sf::Packet packet;
        packet << PacketType::TRANSFORM << tData;

        if (states[0].ready && !states[0].disconnected)
            socket.send(packet, states[0].ip, states[0].port);
        if (states[1].ready && !states[1].disconnected)
            socket.send(packet, states[1].ip, states[1].port);
    }
}

void GameSession::SendToPlayer(int playerId, sf::Packet& packet)
{
    int idx = GetIndex(playerId);
    if (idx == -1 || !states[idx].ready || states[idx].disconnected)
        return;

    std::lock_guard<std::mutex> lock(socketMutex);
    socket.send(packet, states[idx].ip, states[idx].port);
}

void GameSession::SendToOther(int playerId, sf::Packet& packet)
{
    int otherId = GetOtherPlayerId(playerId);
    int idx = GetIndex(otherId);
    if (idx == -1 || !states[idx].ready || states[idx].disconnected)
        return;

    std::lock_guard<std::mutex> lock(socketMutex);
    socket.send(packet, states[idx].ip, states[idx].port);
}

void GameSession::UpdateBullets(float dt)
{
    if (dt <= 0.f)
        return;

    std::vector<int> hitOwners;

    {
        std::lock_guard<std::mutex> lock(bulletsMutex);

        for (BulletState& bullet : bullets)
        {
            if (!bullet.active)
                continue;

            sf::Vector2f previousPos = bullet.position;
            sf::Vector2f delta = bullet.velocity * dt;
            bullet.position += delta;
            bullet.traveled += std::sqrt(delta.x * delta.x + delta.y * delta.y);

            if (SegmentHitsWall(previousPos, bullet.position))
            {
                bullet.active = false;
                std::cout << "[UDP-MISS] Room " << roomId
                    << " shooter: " << bullet.ownerID
                    << " reason: wall"
                    << std::endl;
                continue;
            }

            if (bullet.traveled >= BULLET_MAX_DIST)
            {
                bullet.active = false;
                std::cout << "[UDP-MISS] Room " << roomId
                    << " shooter: " << bullet.ownerID
                    << " reason: range"
                    << std::endl;
                continue;
            }

            int targetPlayerId = GetOtherPlayerId(bullet.ownerID);
            if (targetPlayerId != -1 && SegmentHitsPlayer(previousPos, bullet.position, targetPlayerId))
            {
                bullet.active = false;
                hitOwners.push_back(bullet.ownerID);
            }
        }

        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                [](const BulletState& bullet) { return !bullet.active; }),
            bullets.end()
        );
    }

    for (int ownerId : hitOwners)
    {
        if (finished)
            return;
        HandleHit(ownerId);
    }
}

void GameSession::LoadCollisionMap()
{
    static const char* mapPaths[] = {
        "Tilemaps/Tilemap1.txt",
        "SFML_CLIENT/SFML_CLIENT/Tilemaps/Tilemap1.txt",
        "../SFML_CLIENT/SFML_CLIENT/Tilemaps/Tilemap1.txt",
        "../../SFML_CLIENT/SFML_CLIENT/Tilemaps/Tilemap1.txt",
        "../../../SFML_CLIENT/SFML_CLIENT/Tilemaps/Tilemap1.txt",
        "../../../../SFML_CLIENT/SFML_CLIENT/Tilemaps/Tilemap1.txt"
    };

    for (const char* path : mapPaths)
    {
        std::ifstream file(path);
        if (!file.is_open())
            continue;

        std::vector<std::string> loadedRows;
        std::string row;
        while (std::getline(file, row))
        {
            if (!row.empty())
                loadedRows.push_back(row);
        }

        if (!loadedRows.empty())
        {
            mapRows = loadedRows;
            std::cout << "[UDP-MAP] Collision map loaded from " << path
                << " rows: " << mapRows.size()
                << std::endl;
            return;
        }
    }

    mapRows = {
        "################",
        "#____________#_#",
        "#______________#",
        "#____#_________#",
        "#_________#____#",
        "#__#___________#",
        "#______#_______#",
        "#__#________#__#",
        "################"
    };

    std::cout << "[UDP-MAP] Using embedded fallback collision map." << std::endl;
}

bool GameSession::IsWallAt(const sf::Vector2f& position) const
{
    if (mapRows.empty())
        return false;

    if (position.x < 0.f || position.y < 0.f)
        return true;

    int row = static_cast<int>(std::floor(position.y / TILE_SIZE));
    int col = static_cast<int>(std::floor(position.x / TILE_SIZE));

    if (row < 0 || row >= static_cast<int>(mapRows.size()))
        return true;
    if (col < 0 || col >= static_cast<int>(mapRows[row].size()))
        return true;

    return mapRows[row][col] == '#';
}

bool GameSession::SegmentHitsWall(const sf::Vector2f& from, const sf::Vector2f& to) const
{
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float longest = std::max(std::abs(dx), std::abs(dy));
    int steps = std::max(1, static_cast<int>(std::ceil(longest / PROJECTILE_STEP)));

    for (int i = 0; i <= steps; i++)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        sf::Vector2f point(from.x + dx * t, from.y + dy * t);
        if (IsWallAt(point))
            return true;
    }

    return false;
}

bool GameSession::PointHitsPlayer(const sf::Vector2f& point, int targetPlayerId) const
{
    int targetIndex = GetIndex(targetPlayerId);
    if (targetIndex == -1)
        return false;

    const PlayerState& target = states[targetIndex];
    if (!target.ready || target.disconnected)
        return false;

    float dx = std::abs(point.x - target.position.x);
    float dy = std::abs(point.y - target.position.y);

    return dx <= (PLAYER_HIT_HALF_WIDTH + BULLET_HIT_RADIUS)
        && dy <= (PLAYER_HIT_HALF_HEIGHT + BULLET_HIT_RADIUS);
}

bool GameSession::SegmentHitsPlayer(const sf::Vector2f& from, const sf::Vector2f& to, int targetPlayerId) const
{
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float longest = std::max(std::abs(dx), std::abs(dy));
    int steps = std::max(1, static_cast<int>(std::ceil(longest / PROJECTILE_STEP)));

    for (int i = 0; i <= steps; i++)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        sf::Vector2f point(from.x + dx * t, from.y + dy * t);
        if (PointHitsPlayer(point, targetPlayerId))
            return true;
    }

    return false;
}

void GameSession::HandleHit(int shooterPlayerId)
{
    int rivalIndex = (playerIds[0] == shooterPlayerId) ? 1 : 0;
    PlayerState& rival = states[rivalIndex];

    rival.health--;

    std::cout << "[UDP-HIT] Room " << roomId
        << " shooter: " << shooterPlayerId
        << " target: " << playerIds[rivalIndex]
        << " health: " << rival.health
        << " lifes: " << rival.lifes
        << std::endl;

    if (rival.health <= 0)
    {
        rival.lifes--;
        std::cout << "[UDP-LIFE] Player " << playerIds[rivalIndex]
            << " lost a life. Lifes left: " << rival.lifes
            << std::endl;

        if (rival.lifes <= 0)
        {
            FinishGame(shooterPlayerId, false);
            return;
        }
        RespawnPlayer(playerIds[rivalIndex]);
    }

    PlayerHitData hitData;
    hitData.targetPlayerId = playerIds[rivalIndex];
    hitData.newHealth = rival.health;
    hitData.newLifes = rival.lifes;
    hitData.respawnPosition = rival.position;

    sf::Packet hitPacket;
    hitPacket << PacketType::PLAYER_HIT << hitData;

    {
        std::lock_guard<std::mutex> lock(socketMutex);
        if (states[0].ready && !states[0].disconnected)
            socket.send(hitPacket, states[0].ip, states[0].port);
        if (states[1].ready && !states[1].disconnected)
            socket.send(hitPacket, states[1].ip, states[1].port);
    }
}

void GameSession::RespawnPlayer(int playerId)
{
    PlayerState& state = GetState(playerId);
    state.health = MAX_HEALTH;
    state.position = sf::Vector2f(RESPAWN_X, RESPAWN_Y);
    state.velocity = sf::Vector2f(0.f, 0.f);

    std::cout << "[UDP-RESPAWN] Player " << playerId
        << " pos: " << state.position.x << ", " << state.position.y
        << " health: " << state.health
        << " lifes: " << state.lifes
        << std::endl;
}

void GameSession::PredictPositions(float dt)
{
    for (int i = 0; i < 2; i++)
    {
        PlayerState& state = states[i];
        float timeSincePacket = state.lastPacketClock.getElapsedTime().asSeconds();

        if (timeSincePacket > PREDICT_TIMEOUT)
            state.position += state.velocity * dt;
    }
}

void GameSession::CheckDisconnects()
{
    for (int i = 0; i < 2; i++)
    {
        if (!states[i].ready || states[i].disconnected)
            continue;

        float withoutPackets = states[i].lastPacketClock.getElapsedTime().asSeconds();
        if (withoutPackets < DISCONNECT_TIMEOUT)
            continue;

        std::cout << "Player " << playerIds[i] << " timeout in room " << roomId << std::endl;
        DisconnectPlayer(playerIds[i]);
        return;
    }
}

void GameSession::CheckHelloTimeout()
{
    if (sessionClock.getElapsedTime().asSeconds() < HELLO_TIMEOUT)
        return;

    if (states[0].ready && !states[1].ready)
    {
        std::cout << "Player " << playerIds[1] << " never joined UDP in room " << roomId << std::endl;
        DisconnectPlayer(playerIds[1]);
    }
    else if (states[1].ready && !states[0].ready)
    {
        std::cout << "Player " << playerIds[0] << " never joined UDP in room " << roomId << std::endl;
        DisconnectPlayer(playerIds[0]);
    }
    else if (!states[0].ready && !states[1].ready)
    {
        std::cout << "Room " << roomId << " closed because nobody joined UDP" << std::endl;
        finished = true;
    }
}

void GameSession::SendPlayerDisconnected(int playerId)
{
    sf::Packet packet;
    packet << PacketType::PLAYER_DISCONNECTED << playerId;

    std::lock_guard<std::mutex> lock(socketMutex);
    if (states[0].ready)
        socket.send(packet, states[0].ip, states[0].port);
    if (states[1].ready)
        socket.send(packet, states[1].ip, states[1].port);
}

void GameSession::FinishGame(int winnerPlayerId, bool cheating)
{
    if (finished)
        return;

    finished = true;

    int winnerIndex = GetIndex(winnerPlayerId);
    if (winnerIndex == -1)
        return;

    int loserIndex = (winnerIndex == 0) ? 1 : 0;
    int loserPlayerId = playerIds[loserIndex];

    EndgameData endData;
    endData.winnerPlayerId = winnerPlayerId;
    endData.loserPlayerId = loserPlayerId;
    endData.cheating = cheating;

    sf::Packet endPacket;
    endPacket << PacketType::ENDGAME << endData;

    {
        std::lock_guard<std::mutex> lock(socketMutex);
        if (states[0].ready)
            socket.send(endPacket, states[0].ip, states[0].port);
        if (states[1].ready)
            socket.send(endPacket, states[1].ip, states[1].port);
    }

    // El ranking lo reporta el servidor UDP, no el cliente.
    if (IsRankedRoomId(roomId))
    {
        ReportGameResult(winnerPlayerId, loserPlayerId);
    }
    else
    {
        std::cout << "[UDP-END] Room " << roomId
            << " amistosa: no se actualiza ranking."
            << std::endl;
    }

    std::cout << "[UDP-END] Room " << roomId
        << " winner: " << winnerPlayerId
        << " loser: " << loserPlayerId
        << " cheating: " << cheating
        << std::endl;
}

void GameSession::ReportGameResult(int winnerPlayerId, int loserPlayerId)
{
    if (!IsRankedRoomId(roomId))
        return;

    int winnerIndex = GetIndex(winnerPlayerId);
    int loserIndex = GetIndex(loserPlayerId);

    if (winnerIndex == -1 || loserIndex == -1)
        return;

    Result winnerResult;
    winnerResult.username = playerNames[winnerIndex];
    winnerResult.scoredPoints = WIN_POINTS;

    Result loserResult;
    loserResult.username = playerNames[loserIndex];
    loserResult.scoredPoints = LOSE_POINTS;

    GameResultData resultData;
    resultData.roomId = roomId;
    resultData.results.push_back(winnerResult);
    resultData.results.push_back(loserResult);

    // Avisamos al servidor TCP local, que es el que toca la base de datos.
    auto serverIp = sf::IpAddress::resolve(RANKING_SERVER_IP);
    if (!serverIp.has_value())
    {
        std::cout << "[UDP-END] No se pudo resolver el servidor de ranking." << std::endl;
        return;
    }

    sf::TcpSocket tcpSocket;
    if (tcpSocket.connect(*serverIp, RANKING_SERVER_PORT) != sf::Socket::Status::Done)
    {
        std::cout << "[UDP-END] No se pudo conectar al servidor de ranking." << std::endl;
        return;
    }

    // El Game Server es quien decide el resultado real de la partida.
    sf::Packet packet;
    packet << PacketType::ENDGAME << resultData;

    if (tcpSocket.send(packet) != sf::Socket::Status::Done)
        std::cout << "[UDP-END] No se pudo enviar el resultado al ranking." << std::endl;
    else
        std::cout << "[UDP-END] Resultado enviado al ranking: "
                  << winnerResult.username << " +" << WIN_POINTS
                  << ", " << loserResult.username << " " << LOSE_POINTS
                  << std::endl;

    tcpSocket.disconnect();
}

PlayerState& GameSession::GetState(int playerId)
{
    return (playerIds[0] == playerId) ? states[0] : states[1];
}

int GameSession::GetIndex(int playerId) const
{
    if (playerIds[0] == playerId)
        return 0;
    if (playerIds[1] == playerId)
        return 1;
    return -1;
}

int GameSession::GetOtherPlayerId(int playerId) const
{
    int idx = GetIndex(playerId);
    if (idx == -1)
        return -1;

    return playerIds[(idx == 0) ? 1 : 0];
}
