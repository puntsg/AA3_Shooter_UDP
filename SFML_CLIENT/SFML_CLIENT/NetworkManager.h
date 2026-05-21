#pragma once

#include <SFML/Network.hpp>
#include <string>
#include <vector>
#include <memory>
#include "PacketTypes.h"
#include "ProtocolData.h"
#include "ClientState.h"

#define NM NetworkManager::Instance()

class NetworkManager
{
public:
    inline static NetworkManager& Instance()
    {
        static NetworkManager nm;
        return nm;
    }

    // --- Conexion con el servidor bootstrap ---
    bool Connect(const sf::IpAddress& serverIp, unsigned short serverPort);
    bool ConnectToServer();       // Usa SERVER_IP y SERVER_PORT por defecto
    void CloseConnection();
    void DisconnectFromServer();  // Alias de CloseConnection

    // --- Protocolo de sala ---
    void SendCreateRoomRequest(const std::string& roomId, const std::string& nickname, unsigned short gamePort);
    void SendJoinRoomRequest(const std::string& roomId, const std::string& nickname, unsigned short gamePort);
    void SendMatchmakingRequest(bool ranked, const std::string& nickname, unsigned short gamePort);

    void ReceiveData();
    void NetworkFetch();          // Alias de ReceiveData

    bool IsConnected() const;
    const ClientState& GetClientState() const;
    ClientState& GetClientState();

    // --- Conexiones P2P (juego) ---
    bool StartP2PListener(unsigned short port);
    void AcceptPeerConnections();
    void AddConnection(const std::string& ip, unsigned short port);
    void SendToAllConnections(sf::Packet& packet);
    const std::vector<std::unique_ptr<sf::TcpSocket>>& GetConnections() const;
    std::vector<std::unique_ptr<sf::TcpSocket>>& GetConnections();
    void ClearConnections();
    void SendToServer(sf::Packet& packet);
    bool SendLoginRequest(const std::string& username, const std::string& password);
    bool SendRegisterRequest(const std::string& username, const std::string& password);
    bool SendRankingRequest(const std::string& username);
    void NotifyPlayerWin(const std::string& username);

private:
    NetworkManager();
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    void ProcessPacket(sf::Packet& packet);
    void HandleCreateRoomResponse(sf::Packet& packet);
    void HandleJoinRoomResponse(sf::Packet& packet);
    void HandleRoomStatusUpdate(sf::Packet& packet);
    void HandleStartGame(sf::Packet& packet);
    void HandleErrorMessage(sf::Packet& packet);
    void HandleLoginResponse(sf::Packet& packet);
    void HandleRegisterResponse(sf::Packet& packet);
    void HandleRankingResponse(sf::Packet& packet);



    sf::TcpSocket m_socket;
    bool m_isConnected;
    ClientState m_clientState;
    sf::TcpListener* listener;

    std::vector<std::unique_ptr<sf::TcpSocket>> m_gameConnections;

    static constexpr unsigned short SERVER_PORT = 55000;
    const sf::IpAddress SERVER_IP = sf::IpAddress(127, 0, 0, 1);
};
