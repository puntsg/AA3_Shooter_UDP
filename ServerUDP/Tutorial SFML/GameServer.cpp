#include "GameServer.h"
#include <iostream>
#include <chrono>

GameServer::GameServer()
    : pool(static_cast<int>(std::thread::hardware_concurrency()))
    , running(false)
{
}

GameServer::~GameServer()
{
    Stop();
}

bool GameServer::Start()
{
    if (udpSocket.bind(UDP_GAME_PORT) != sf::Socket::Status::Done)
    {
        std::cerr << "Fail bind UDP " << UDP_GAME_PORT << std::endl;
        return false;
    }
    udpSocket.setBlocking(true);

    if (tcpListener.listen(TCP_LISTEN_PORT) != sf::Socket::Status::Done)
    {
        std::cerr << "Fail listen TCP " << TCP_LISTEN_PORT << std::endl;
        return false;
    }

    running = true;

    std::cout << "TCP ON: " << TCP_LISTEN_PORT
        << " UDP ON: " << UDP_GAME_PORT
        << " Threads Num: " << pool.Size()
        << std::endl;

    tcpThread = std::thread(&GameServer::TcpListenerLoop, this);
    udpThread = std::thread(&GameServer::UdpReceiveLoop, this);
    updateThread = std::thread(&GameServer::UpdateLoop, this);

    return true;
}

void GameServer::Stop()
{
    running = false;
    udpSocket.unbind();
    tcpListener.close();

    if (tcpThread.joinable()) tcpThread.join();
    if (udpThread.joinable()) udpThread.join();
    if (updateThread.joinable()) updateThread.join();
}

void GameServer::TcpListenerLoop()
{
    tcpListener.setBlocking(true);

    while (running)
    {
        sf::TcpSocket bootstrapSocket;
        if (tcpListener.accept(bootstrapSocket) != sf::Socket::Status::Done)
            continue;

        std::cout << "TCP link gotten" << std::endl;

        sf::Packet packet;
        if (bootstrapSocket.receive(packet) == sf::Socket::Status::Done)
        {
            PacketType type = NONE;
            packet >> type;

            if (type == START_GAME)
                HandleSessionStart(packet);
        }

        bootstrapSocket.disconnect();
    }
}

void GameServer::UdpReceiveLoop()
{
    while (running)
    {
        sf::Packet packet;
        std::optional<sf::IpAddress> senderIp;
        unsigned short senderPort;

        if (udpSocket.receive(packet, senderIp, senderPort) == sf::Socket::Status::Done)
        {
            if (!senderIp.has_value())
                continue;

            sf::IpAddress ip = senderIp.value();

            // metemos el packet a procesar
            pool.Enqueue([this, ip, senderPort, packet]() mutable
                {
                    RouteUdpPacket(ip, senderPort, packet);
                });
        }
    }
}

void GameServer::UpdateLoop()
{
    sf::Clock clock;

    while (running)
    {
        float dt = clock.restart().asSeconds();

        {
            std::lock_guard<std::mutex> lock(sessionsMutex);
            for (std::pair<const std::string, std::shared_ptr<GameSession>>& pair : sessions)
                pair.second->Update(dt);

            CleanFinishedSessions();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void GameServer::HandleSessionStart(sf::Packet& packet)
{
    SessionStartData sessionData;
    packet >> sessionData;

    if (sessionData.players.size() < 2)
        return;

    std::shared_ptr<GameSession> session = std::make_shared<GameSession>(
        sessionData.roomId,
        sessionData.players[0],
        sessionData.players[1],
        udpSocket
    );

    {
        std::lock_guard<std::mutex> lock(sessionsMutex);
        sessions[sessionData.roomId] = session;
    }

    std::cout << "Room created: " << sessionData.roomId
        << " Total rooms: " << sessions.size() << std::endl;
}

void GameServer::RouteUdpPacket(const sf::IpAddress& senderIp, unsigned short senderPort, sf::Packet& packet)
{
    PacketType type = NONE;
    packet >> type;

    std::shared_ptr<GameSession> session = nullptr;
    int playerId = -1;

    {
        std::lock_guard<std::mutex> lock(sessionsMutex);
        for (std::pair<const std::string, std::shared_ptr<GameSession>>& pair : sessions)
        {
            if (pair.second->BelongsToSession(senderIp, senderPort))
            {
                session = pair.second;
                playerId = pair.second->GetPlayerIdByAddress(senderIp, senderPort);
                break;
            }
        }
    }

    if (session == nullptr || playerId == -1)
        return;

    switch (type)
    {
    case TRANSFORM:
        session->ProcessMovePacket(playerId, packet);
        break;
    case SHOOT:
        session->ProcessShotPacket(playerId, packet);
        break;
    case PLAYER_TAUNT:
        session->ProcessTauntPacket(playerId);
        break;
    default:
        break;
    }
}

void GameServer::CleanFinishedSessions()
{
    for (std::map<std::string, std::shared_ptr<GameSession>>::iterator it = sessions.begin(); it != sessions.end(); )
    {
        if (it->second->IsFinished())
        {
            std::cout << "Room deleted: " << it->first << std::endl;
            it = sessions.erase(it);
        }
        else
            ++it;
    }
}