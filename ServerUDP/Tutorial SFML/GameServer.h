#pragma once

#include <SFML/Network.hpp>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <string>
#include "ThreadPool.h"
#include "GameSession.h"
#include "ProtocolData.h"
#include "PacketTypes.h"

//Inventados eh
#define TCP_LISTEN_PORT 55001
#define UDP_GAME_PORT 55002

class GameServer
{
public:
    GameServer();
    ~GameServer();

    bool Start();
    void Stop();

private:
    void TcpListenerLoop();
    void UdpReceiveLoop();
    void UpdateLoop();

    bool HandleSessionStart(sf::Packet& packet, SessionStartResponseData& response);
    void RouteUdpPacket(const sf::IpAddress& senderIp, unsigned short senderPort, sf::Packet& packet);
    void CleanFinishedSessions();

    sf::TcpListener tcpListener;
    sf::UdpSocket udpSocket;

    std::map<std::string, std::shared_ptr<GameSession>> sessions;
    std::mutex sessionsMutex;

    ThreadPool pool;

    std::thread tcpThread;
    std::thread udpThread;
    std::thread updateThread;

    bool running;
};
