#pragma once

#include <SFML/Network.hpp>
#include <string>
#include <vector>
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
    void SendCreateRoomRequest(const std::string& roomId, const std::string& nickname);
    void SendJoinRoomRequest(const std::string& roomId, const std::string& nickname);
    bool SendMatchmakingRequest(bool ranked, const std::string& nickname);
    bool SendCancelMatchmakingRequest();

    void ReceiveData();
    void NetworkFetch();          // Alias de ReceiveData

    bool IsConnected() const;
    const ClientState& GetClientState() const;
    ClientState& GetClientState();

    void ClearGameNetworkState();
    bool SendUdpHelloReady();

    // Envia paquete UDP al GameServer
    void SendUdp(sf::Packet& packet);

    // Procesa paquetes UDP pendientes al GS
    void ReceiveUdpData();

    void SendToServer(sf::Packet& packet);
    bool SendLoginRequest(const std::string& username, const std::string& password);
    bool SendRegisterRequest(const std::string& username, const std::string& password);
    bool SendRankingRequest(const std::string& username);

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

    // Handlers UDP
    void HandleTransform(sf::Packet& packet);
    void HandleShootReplicate(sf::Packet& packet);
    void HandlePlayerHit(sf::Packet& packet);
    void HandlePlayerTaunt(sf::Packet& packet);
    void HandleEndgame(sf::Packet& packet);



    sf::TcpSocket m_socket;
    sf::UdpSocket m_udpSocket;
    bool m_isConnected;
    bool m_udpSocketReady;
    ClientState m_clientState;
};
