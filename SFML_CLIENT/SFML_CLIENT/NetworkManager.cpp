#include "NetworkManager.h"
#include "Constants.h"
#include <iostream>
#include <optional>

NetworkManager::NetworkManager()
    : m_isConnected(false)
    , m_udpSocketReady(false)
{
}

bool NetworkManager::ConnectToServer()
{
    if (m_isConnected)
    {
        std::cout << "Ya conectado al servidor" << std::endl;
        return true;
    }

    const char* ips[] = {
        Config::Connection::SERVER_IP,
        Config::Connection::SERVER_IP_LAN
    };

    for (int i = 0; i < 2; i++)
    {
        std::optional<sf::IpAddress> serverIp = sf::IpAddress::resolve(ips[i]);
        if (!serverIp.has_value())
        {
            std::cerr << "[CLIENT] IP del servidor invalida: " << ips[i] << std::endl;
            continue;
        }

        if (Connect(*serverIp, Config::Connection::SERVER_PORT))
        {
            return true;
        }

        m_socket.disconnect();
    }

    return false;
}

void NetworkManager::DisconnectFromServer()
{
    CloseConnection();
}

void NetworkManager::NetworkFetch()
{
    ReceiveData();
}

void NetworkManager::ReceiveData()
{
    if (m_isConnected)
    {
        sf::Packet packet;
        sf::Socket::Status status = m_socket.receive(packet);
        while (status == sf::Socket::Status::Done)
        {
            ProcessPacket(packet);
            packet.clear();
            status = m_socket.receive(packet);
        }
        if (status == sf::Socket::Status::Disconnected)
        {
            std::cout << "[CLIENT] El servidor ha cerrado la conexion." << std::endl;
            m_isConnected = false;
        }
    }

}

void NetworkManager::ClearGameNetworkState()
{
    if (m_udpSocketReady)
    {
        m_udpSocket.unbind();
        m_udpSocketReady = false;
    }

    std::cout << "[CLIENT-UDP] Socket de partida cerrado." << std::endl;
}

bool NetworkManager::SendUdpHelloReady()
{
    std::optional<sf::IpAddress> ip = sf::IpAddress::resolve(m_clientState.gameServerIp);
    if (!ip.has_value())
    {
        std::cerr << "[CLIENT-UDP] IP del GameServer invalida: "
            << m_clientState.gameServerIp
            << std::endl;
        return false;
    }

    if (!m_udpSocketReady)
    {
        // puerto libre
        if (m_udpSocket.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done)
        {
            std::cerr << "[CLIENT-UDP] No se pudo abrir un puerto UDP local." << std::endl;
            return false;
        }

        m_udpSocket.setBlocking(false);
        m_udpSocketReady = true;
    }

    UdpHelloData helloData;
    helloData.roomId = m_clientState.currentRoomId;
    helloData.playerId = m_clientState.playerId;

    sf::Packet packet;
    packet << PacketType::UDP_HELLO << helloData;

    if (m_udpSocket.send(packet, *ip, m_clientState.gameServerUdpPort) != sf::Socket::Status::Done)
    {
        std::cerr << "[CLIENT-UDP] Error enviando UDP_HELLO." << std::endl;
        return false;
    }

    std::cout << "[CLIENT-UDP] hello -> "
        << m_clientState.gameServerIp << ":" 
        << m_clientState.gameServerUdpPort 
        << " puerto local "
        << m_udpSocket.getLocalPort()
        << std::endl;

    return true;
}

bool NetworkManager::Connect(const sf::IpAddress& serverIp, unsigned short serverPort)
{
    m_socket.setBlocking(true); 
    if (m_socket.connect(serverIp, serverPort) != sf::Socket::Status::Done)
    {
        std::cerr << "[CLIENT] Error al conectar con el servidor." << std::endl;
        m_isConnected = false;
        return false;
    }

    m_socket.setBlocking(false);
    m_isConnected = true;

    std::cout << "[CLIENT] Conectado al servidor "
        << serverIp.toString()
        << ":"
        << serverPort
        << std::endl;

    return true;
}

void NetworkManager::CloseConnection()
{
    if (!m_isConnected)
    {
        return;
    }

    m_socket.disconnect();
    m_isConnected = false;

    std::cout << "[CLIENT] Conexion cerrada correctamente." << std::endl;
}

void NetworkManager::SendCreateRoomRequest(const std::string& roomId, const std::string& nickname, unsigned short gamePort)
{
    if (!m_isConnected)
    {
        std::cerr << "[CLIENT] No se puede enviar CREATE_ROOM_REQUEST: no hay conexion." << std::endl;
        return;
    }

    sf::Packet packet;

    CreateRoomRequestData requestData;
    requestData.roomId = roomId;
    requestData.username = nickname;
    requestData.gamePort = gamePort;

    packet << static_cast<short>(PacketType::CREATE_ROOM_REQUEST);
    packet << requestData;

    if (m_socket.send(packet) == sf::Socket::Status::Done)
    {
        std::cout << "[CLIENT] CREATE_ROOM_REQUEST enviada. RoomId: " << roomId << std::endl;
    }
    else
    {
        std::cerr << "[CLIENT] Error al enviar CREATE_ROOM_REQUEST." << std::endl;
    }
}

void NetworkManager::SendJoinRoomRequest(const std::string& roomId, const std::string& nickname, unsigned short gamePort)
{
    if (!m_isConnected)
    {
        std::cerr << "[CLIENT] No se puede enviar JOIN_ROOM_REQUEST: no hay conexion." << std::endl;
        return;
    }

    sf::Packet packet;

    JoinRoomRequestData requestData;
    requestData.roomId = roomId;
    requestData.username = nickname;
    requestData.gamePort = gamePort;

    packet << static_cast<short>(PacketType::JOIN_ROOM_REQUEST);
    packet << requestData;

    if (m_socket.send(packet) == sf::Socket::Status::Done)
    {
        std::cout << "[CLIENT] JOIN_ROOM_REQUEST enviada. RoomId: " << roomId << std::endl;
    }
    else
    {
        std::cerr << "[CLIENT] Error al enviar JOIN_ROOM_REQUEST." << std::endl;
    }
}

bool NetworkManager::SendMatchmakingRequest(bool ranked, const std::string& nickname, unsigned short gamePort)
{
    if (!m_isConnected)
    {
        std::cerr << "[CLIENT] No se puede enviar MATCHMAKING_REQUEST: no hay conexion." << std::endl;
        return false;
    }

    const std::string queueId = ranked ? "__queue_ranked" : "__queue_normal";
    sf::Packet packet;

    CreateRoomRequestData requestData;
    requestData.roomId = queueId;
    requestData.username = nickname;
    requestData.gamePort = gamePort;

    packet << static_cast<short>(PacketType::CREATE_ROOM_REQUEST);
    packet << requestData;

    if (m_socket.send(packet) != sf::Socket::Status::Done)
    {
        std::cerr << "[CLIENT] Error al enviar MATCHMAKING_REQUEST." << std::endl;
        return false;
    }

    m_clientState.isSearchingMatch = true;
    m_clientState.searchingRanked = ranked;
    m_clientState.isWaitingInRoom = false;
    m_clientState.currentRoomId = queueId;
    m_clientState.roomPlayers.clear();

    return true;
}

bool NetworkManager::SendCancelMatchmakingRequest()
{
    if (!m_isConnected)
    {
        return false;
    }

    sf::Packet packet;
    packet << static_cast<short>(PacketType::DISCONNECT);

    if (m_socket.send(packet) != sf::Socket::Status::Done)
    {
        return false;
    }

    m_clientState.isSearchingMatch = false;
    m_clientState.searchingRanked = false;
    m_clientState.isWaitingInRoom = false;
    m_clientState.currentRoomId.clear();
    m_clientState.roomPlayers.clear();

    return true;
}


bool NetworkManager::IsConnected() const
{
    return m_isConnected;
}

const ClientState& NetworkManager::GetClientState() const
{
    return m_clientState;
}

ClientState& NetworkManager::GetClientState()
{
    return m_clientState;
}

void NetworkManager::ProcessPacket(sf::Packet& packet)
{
    PacketType packetType = PacketType::NONE;
    packet >> packetType;

    switch (packetType)
    {
    case PacketType::LOGIN_RESPONSE:
        HandleLoginResponse(packet);
        break;
    
    case PacketType::REGISTER_RESPONSE:
        HandleRegisterResponse(packet);
        break;

    case PacketType::CREATE_ROOM_RESPONSE:
        HandleCreateRoomResponse(packet);
        break;

    case PacketType::JOIN_ROOM_RESPONSE:
        HandleJoinRoomResponse(packet);
        break;

    case PacketType::ROOM_STATUS_UPDATE:
        HandleRoomStatusUpdate(packet);
        break;

    case PacketType::START_GAME:
        HandleStartGame(packet);
        break;

    case PacketType::ERROR_MESSAGE:
        HandleErrorMessage(packet);
        break;

    case PacketType::RANKING_RESPONSE:
        HandleRankingResponse(packet);
        break;

    default:
        std::cout << "[CLIENT] Paquete recibido no gestionado." << std::endl;
        break;
    }
}

void NetworkManager::SendToServer(sf::Packet& packet)
{
    if (m_socket.send(packet) != sf::Socket::Status::Done)
        std::cerr << "[CLIENT] Error enviando paquete al servidor." << std::endl;
}

bool NetworkManager::SendLoginRequest(const std::string& username, const std::string& password)
{
    if (!m_isConnected)
    {
        m_clientState.authMessage = "No hay conexion con el servidor.";
        m_clientState.authMessageIsError = true;
        std::cerr << "[CLIENT] No se puede enviar LOGIN_REQUEST: no hay conexion." << std::endl;
        return false;
    }

    LoginRequestData loginRequestData;
    loginRequestData.username = username;
    loginRequestData.password = password;

    sf::Packet packet;
    packet << static_cast<short>(PacketType::LOGIN_REQUEST);
    packet << loginRequestData;

    if (m_socket.send(packet) != sf::Socket::Status::Done)
    {
        m_clientState.authMessage = "Error enviando login.";
        m_clientState.authMessageIsError = true;
        return false;
    }

    return true;
}

bool NetworkManager::SendRegisterRequest(const std::string& username, const std::string& password)
{
    if (!m_isConnected)
    {
        m_clientState.authMessage = "No hay conexion con el servidor.";
        m_clientState.authMessageIsError = true;
        std::cerr << "[CLIENT] No se puede enviar REGISTER_REQUEST: no hay conexion." << std::endl;
        return false;
    }

    RegisterRequestData registerRequestData;
    registerRequestData.username = username;
    registerRequestData.password = password;

    sf::Packet packet;
    packet << static_cast<short>(PacketType::REGISTER_REQUEST);
    packet << registerRequestData;

    if (m_socket.send(packet) != sf::Socket::Status::Done)
    {
        m_clientState.authMessage = "Error enviando registro.";
        m_clientState.authMessageIsError = true;
        return false;
    }

    return true;
}

bool NetworkManager::SendRankingRequest(const std::string& username)
{
    if (!m_isConnected)
    {
        m_clientState.rankingLoading = false;
        m_clientState.rankingReceived = true;
        m_clientState.rankingMessage = "No hay conexion con el servidor.";
        m_clientState.rankingMessageIsError = true;
        return false;
    }

    RankingRequestData requestData;
    requestData.username = username;

    sf::Packet packet;
    packet << static_cast<short>(PacketType::RANKING_REQUEST);
    packet << requestData;
    if (m_socket.send(packet) != sf::Socket::Status::Done)
    {
        m_clientState.rankingLoading = false;
        m_clientState.rankingReceived = true;
        m_clientState.rankingMessage = "Error pidiendo ranking.";
        m_clientState.rankingMessageIsError = true;
        return false;
    }

    return true;
}

void NetworkManager::SendUdp(sf::Packet& packet)
{
    if (!m_udpSocketReady)
    {
        std::cerr << "[CLIENT-UDP] Socket no listo para enviar." << std::endl;
        return;
    }

    std::optional<sf::IpAddress> ip = sf::IpAddress::resolve(m_clientState.gameServerIp);
    if (!ip.has_value())
    {
        std::cerr << "[CLIENT-UDP] IP del GameServer invalida." << std::endl;
        return;
    }

    if (m_udpSocket.send(packet, *ip, m_clientState.gameServerUdpPort) != sf::Socket::Status::Done)
        std::cerr << "[CLIENT-UDP] Error enviando paquete UDP." << std::endl;
}

void NetworkManager::ReceiveUdpData()
{
    if (!m_udpSocketReady)
        return;

    sf::Packet packet;
    std::optional<sf::IpAddress> senderIp;
    unsigned short senderPort = 0;

    while (m_udpSocket.receive(packet, senderIp, senderPort) == sf::Socket::Status::Done)
    {
        PacketType type = NONE;
        packet >> type;

        switch (type)
        {
        case PacketType::TRANSFORM:
            HandleTransform(packet);
            break;
        case PacketType::SHOOT_REPLICATE:
            HandleShootReplicate(packet);
            break;
        case PacketType::PLAYER_HIT:
            HandlePlayerHit(packet);
            break;
        case PacketType::PLAYER_TAUNT:
            HandlePlayerTaunt(packet);
            break;
        case PacketType::ENDGAME:
            HandleEndgame(packet);
            break;
        default:
            break;
        }

        packet.clear();
    }
}

void NetworkManager::HandleTransform(sf::Packet& packet)
{
    TransformData data;
    packet >> data;

    for (TransformData& t : m_clientState.incomingTransforms)
    {
        if (t.localPlayerId == data.localPlayerId)
        {
            t = data;
            return;
        }
    }
    m_clientState.incomingTransforms.push_back(data);
}

void NetworkManager::HandleShootReplicate(sf::Packet& packet)
{
    packet >> m_clientState.lastShootReplicate;
    m_clientState.hasShootReplicate = true;
}

void NetworkManager::HandlePlayerHit(sf::Packet& packet)
{
    packet >> m_clientState.lastPlayerHit;
    m_clientState.hasPlayerHit = true;
}

void NetworkManager::HandlePlayerTaunt(sf::Packet& packet)
{
    packet >> m_clientState.tauntPlayerId;
    m_clientState.hasTaunt = true;
}

void NetworkManager::HandleEndgame(sf::Packet& packet)
{
    packet >> m_clientState.endgameData;
    m_clientState.hasEndgame = true;
}

void NetworkManager::HandleCreateRoomResponse(sf::Packet& packet)
{
    CreateRoomResponseData responseData;
    packet >> responseData;

    std::cout << "[CLIENT] CREATE_ROOM_RESPONSE -> "
        << responseData.message
        << std::endl;

    if (responseData.success)
    {
        m_clientState.currentRoomId = responseData.roomId;
        m_clientState.isHost = true;
        m_clientState.isWaitingInRoom = true;
        m_clientState.hasGameStarted = false;
    }
    else
    {
        m_clientState.isSearchingMatch = false;
        m_clientState.searchingRanked = false;
        m_clientState.isWaitingInRoom = false;
        m_clientState.currentRoomId.clear();
        m_clientState.roomPlayers.clear();
    }
}

void NetworkManager::HandleJoinRoomResponse(sf::Packet& packet)
{
    JoinRoomResponseData responseData;
    packet >> responseData;

    std::cout << "[CLIENT] JOIN_ROOM_RESPONSE -> "
        << responseData.message
        << std::endl;

    if (responseData.success)
    {
        m_clientState.currentRoomId = responseData.roomId;
        m_clientState.isHost = false;
        m_clientState.isWaitingInRoom = true;
        m_clientState.hasGameStarted = false;
    }
    else
    {
        m_clientState.isSearchingMatch = false;
        m_clientState.searchingRanked = false;
        m_clientState.isWaitingInRoom = false;
        m_clientState.currentRoomId.clear();
        m_clientState.roomPlayers.clear();
    }
}

void NetworkManager::HandleRoomStatusUpdate(sf::Packet& packet)
{
    RoomStatusUpdateData roomData;
    packet >> roomData;

    std::cout << "[CLIENT] ROOM_STATUS_UPDATE -> Sala "
        << roomData.roomId
        << " | Jugadores: "
        << roomData.currentPlayers
        << "/"
        << roomData.maxPlayers
        << std::endl;

    m_clientState.currentRoomId = roomData.roomId;
    m_clientState.roomPlayers = roomData.players;
    m_clientState.isWaitingInRoom = true;
    m_clientState.isSearchingMatch = roomData.roomId.rfind("__queue_", 0) == 0;

    for (const LobbyPlayerInfo& player : roomData.players)
    {
        std::cout << "  - " << player.username
            << " | host: " << player.isHost
            << " | ip: " << player.ip
            << " | port: " << player.gamePort
            << std::endl;
    }
}

void NetworkManager::HandleStartGame(sf::Packet& packet)
{
    StartGameData startData;
    packet >> startData;

    std::cout << "[CLIENT] START_GAME -> Sala "
        << startData.roomId
        << " | Players: "
        << startData.playerCount
        << " | GameServer UDP: "
        << startData.gameServerIp
        << ":"
        << startData.gameServerUdpPort
        << std::endl;

    m_clientState.currentRoomId = startData.roomId;
    m_clientState.roomPlayers = startData.players;
    m_clientState.gameServerIp = startData.gameServerIp;
    m_clientState.gameServerUdpPort = startData.gameServerUdpPort;
    m_clientState.hasGameStarted = true;
    m_clientState.isWaitingInRoom = false;
    m_clientState.isSearchingMatch = false;
    m_clientState.searchingRanked = false;

    for (const LobbyPlayerInfo& player : startData.players)
    {
        std::cout << "  - " << player.username
            << " | host: " << player.isHost
            << " | ip: " << player.ip
            << " | port: " << player.gamePort
            << std::endl;
    }
    NM.DisconnectFromServer();
}

void NetworkManager::HandleErrorMessage(sf::Packet& packet)
{
    ErrorMessageData errorData;
    packet >> errorData;

    std::cout << "[CLIENT] ERROR_MESSAGE -> "
        << errorData.message
        << std::endl;
}

void NetworkManager::HandleLoginResponse(sf::Packet& packet)
{
    LoginResponseData loginResponseData;
    packet >> loginResponseData;
    m_clientState.authMessage = loginResponseData.message;
    m_clientState.authMessageIsError = !loginResponseData.success;

    if (loginResponseData.success)
    {
        m_clientState.playerId = loginResponseData.playerId;
        m_clientState.nickname = loginResponseData.username;
    }
}

void NetworkManager::HandleRegisterResponse(sf::Packet& packet)
{
    RegisterResponseData registerResponseData;
    packet >> registerResponseData;
    m_clientState.authMessage = registerResponseData.message;
    m_clientState.authMessageIsError = !registerResponseData.success;
}

void NetworkManager::HandleRankingResponse(sf::Packet& packet)
{
    RankingResponseData responseData;
    packet >> responseData;
    m_clientState.ranking = responseData.entries;
    m_clientState.rankingLoading = false;
    m_clientState.rankingReceived = true;
    m_clientState.rankingMessage = responseData.message;
    m_clientState.rankingMessageIsError = !responseData.success;
    std::cout << "[CLIENT] Ranking recibido: " << responseData.entries.size() << " entradas" << std::endl;
}
