#include "GameSession.h"
#include <iostream>
#include <cmath>

GameSession::GameSession(const std::string& roomId, const LobbyPlayerInfo& p1Info, const LobbyPlayerInfo& p2Info, sf::UdpSocket& socket, std::mutex& socketMutex)
    : roomId(roomId)
    , socket(socket)
    , socketMutex(socketMutex)
    , finished(false)
    , bothReady(false)
{
    playerIds[0] = p1Info.playerId;
    playerIds[1] = p2Info.playerId;

    // se llena con el hello udp
    states[0].ip = sf::IpAddress::Any;
    states[0].port = 0;
    states[0].position = sf::Vector2f(P1_START_X, START_Y);

    states[1].ip = sf::IpAddress::Any;
    states[1].port = 0;
    states[1].position = sf::Vector2f(P2_START_X, START_Y);

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
    state.lastValidPacketId = moveData.packetId;
}

void GameSession::ProcessShotPacket(int playerId, sf::Packet& packet)
{
    PlayerState& shooter = GetState(playerId);
    shooter.lastPacketClock.restart();

    std::cout << "[UDP-SHOT] Room " << roomId
        << " shooter: " << playerId
        << " pos: " << shooter.position.x << ", " << shooter.position.y
        << std::endl;

    ShootReplicateData replicateData;
    replicateData.position = shooter.position;
    replicateData.flipped = shooter.flipped;

    sf::Packet replicatePacket;
    replicatePacket << PacketType::SHOOT_REPLICATE << replicateData;
    SendToOther(playerId, replicatePacket);

    HandleHit(playerId);
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

    std::cout << "[UDP-END] Room " << roomId
        << " winner: " << winnerPlayerId
        << " loser: " << loserPlayerId
        << " cheating: " << cheating
        << std::endl;
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
