#pragma once

#include <SFML/Network.hpp>
#include <cstddef>
#include <vector>
#include <memory>
#include <string>
#include <mutex>
#include "ConnectedClient.h"
#include "RoomManager.h"
#include "PacketTypes.h"
#include "ProtocolData.h"
#include "ThreadPool.h"
#define NM NetworkManager::Instance()

class NetworkManager
{
public:
    NetworkManager();
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;
    inline static NetworkManager& Instance()
    {
        static NetworkManager nm;
        return nm;
    }

    bool Start(unsigned short listenPort);
    void Update();

    void ProcessPacketFromPool(int playerId, sf::Packet packet);

private:
    void AcceptNewClients();
    void ReceiveClientData();

    void ProcessPacket(ConnectedClient& client, sf::Packet& packet);

    void HandleCheckMap(ConnectedClient& client, sf::Packet& packet);
    void HandleMapRequest(ConnectedClient& client);

    void HandleRegisterRequest(ConnectedClient& client, sf::Packet& packet);
    void HandleLoginRequest(ConnectedClient& client, sf::Packet& packet);

    void HandleCreateRoomRequest(ConnectedClient& client, sf::Packet& packet);
    void HandleJoinRoomRequest(ConnectedClient& client, sf::Packet& packet);
    void HandleMatchmakingRequest(ConnectedClient& client, const CreateRoomRequestData& requestData, bool ranked);
    void HandleEndGame(ConnectedClient& client, sf::Packet& packet);
    void HandleRankingRequest(ConnectedClient& client, sf::Packet& packet);
    void HandleDisconnectRequest(ConnectedClient& client);

    void SendCreateRoomResponse(ConnectedClient& client, bool success, const std::string& roomId, const std::string& message);
    void SendJoinRoomResponse(ConnectedClient& client, bool success, const std::string& roomId, const std::string& message);
    void SendLoginResponse(ConnectedClient& client, const LoginResponseData& data);
    void SendRegisterResponse(ConnectedClient& client, const RegisterResponseData& data);
    void SendErrorMessage(ConnectedClient& client, const std::string& message);

    bool SendSessionToGameServer(const StartGameData& startData, std::string& message);

    void BroadcastRoomStatus(const std::string& roomId);
    void TryStartGame(const std::string& roomId);
    void TryCreateMatchFromQueue(std::vector<int>& queue, const std::string& queueName);
    void RemoveClientFromMatchmakingQueues(int playerId);

    ConnectedClient* GetClientById(int playerId);
    ConnectedClient* GetClientBySocket(sf::TcpSocket* socket);

    void RemoveDisconnectedClient(std::size_t index);

    void PrintConnectedClients() const;

private:
    sf::TcpListener m_listener;
    bool m_isRunning;
    int m_nextPlayerId;

    // Protege clientes, salas y colas de matchmaking cuando trabajan varios threads.
    std::mutex m_stateMutex;

    std::vector<std::unique_ptr<sf::TcpSocket>> m_sockets;
    std::vector<ConnectedClient> m_clients;
    RoomManager m_roomManager;
    std::vector<std::string> connectedUsers;
    std::vector<int> m_normalQueue;
    std::vector<int> m_rankedQueue;

    // Thread pool sencillo para procesar paquetes del bootstrap server.
    ThreadPool m_threadPool;
};
