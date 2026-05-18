#pragma once

#include <SFML/Network.hpp>
#include <vector>
#include <memory>
#include <string>
#include "ConnectedClient.h"
#include "RoomManager.h"
#include "PacketTypes.h"
#include "ProtocolData.h"
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

private:
    void AcceptNewClients();
    void ReceiveClientData();

    void ProcessPacket(ConnectedClient& client, sf::Packet& packet);

    void HandleRegisterRequest(ConnectedClient& client, sf::Packet& packet);
    void HandleLoginRequest(ConnectedClient& client, sf::Packet& packet);

    void HandleCreateRoomRequest(ConnectedClient& client, sf::Packet& packet);
    void HandleJoinRoomRequest(ConnectedClient& client, sf::Packet& packet);
    void HandleMatchmakingRequest(ConnectedClient& client, const CreateRoomRequestData& requestData, bool ranked);
    void HandleEndGame(ConnectedClient& client, sf::Packet& packet);
    void HandleRankingRequest(ConnectedClient& client, sf::Packet& packet);

    void SendCreateRoomResponse(ConnectedClient& client, bool success, const std::string& roomId, const std::string& message);
    void SendJoinRoomResponse(ConnectedClient& client, bool success, const std::string& roomId, const std::string& message);
    void SendLoginResponse(ConnectedClient& client, const LoginResponseData& data);
    void SendRegisterResponse(ConnectedClient& client, const RegisterResponseData& data);
    void SendErrorMessage(ConnectedClient& client, const std::string& message);

    void BroadcastRoomStatus(const std::string& roomId);
    void TryStartGame(const std::string& roomId);
    void TryCreateMatchFromQueue(std::vector<int>& queue, const std::string& queueName);
    void RemoveClientFromMatchmakingQueues(int playerId);

    void HandleRankingUpdate(ConnectedClient& client, sf::Packet& packet);
    void ProcessRankingValidation(const std::string& roomId);

    ConnectedClient* GetClientById(int playerId);
    ConnectedClient* GetClientBySocket(sf::TcpSocket* socket);

    void RemoveDisconnectedClient(int index);

    void PrintConnectedClients() const;


private:
    sf::TcpListener m_listener;
    bool m_isRunning;
    int m_nextPlayerId;
    std::vector<std::unique_ptr<sf::TcpSocket>> m_sockets;
    std::vector<ConnectedClient> m_clients;
    RoomManager m_roomManager;
    std::map<std::string, std::vector<RankingUpdateData>> pendingRankingUpdates;
    std::vector<std::string> connectedUsers;
    std::vector<int> m_normalQueue;
    std::vector<int> m_rankedQueue;
};
