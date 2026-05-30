#include "NetworkManager.h"
#include <iostream>

NetworkManager::NetworkManager()
    : m_isConnected(false)
    , m_udpSocketReady(false)
    , listener(nullptr)
{
}

bool NetworkManager::ConnectToServer()
{
    if (m_isConnected)
    {
        std::cout << "Ya conectado al servidor" << std::endl;
        return true;
    }
    return Connect(SERVER_IP, SERVER_PORT);
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

    AcceptPeerConnections();
}

bool NetworkManager::StartP2PListener(unsigned short port)
{
    if (listener == nullptr) {
        listener = new sf::TcpListener();
    }
    
    if (listener->listen(port) != sf::Socket::Status::Done) {
        std::cerr << "[CLIENT-P2P] Error al crear listener P2P en puerto " << port << std::endl;
        return false;
    }
    
    listener->setBlocking(false);
    std::cout << "[CLIENT-P2P] Escuchando conexiones P2P en el puerto " << port << std::endl;
    return true;
}

void NetworkManager::AcceptPeerConnections()
{
    if (listener == nullptr) return;

    auto newSocket = std::make_unique<sf::TcpSocket>();
    newSocket->setBlocking(false);

    if (listener->accept(*newSocket) == sf::Socket::Status::Done)
    {
        std::cout << "[CLIENT-P2P] Un peer (rival) se ha conectado!" << std::endl;
        m_gameConnections.push_back(std::move(newSocket));
    }
}

void NetworkManager::AddConnection(const std::string& ip, unsigned short port)
{
    auto newSocket = std::make_unique<sf::TcpSocket>();
    auto address = sf::IpAddress::resolve(ip);
    if (!address)
    {
        std::cerr << "[CLIENT] IP invalida: " << ip << std::endl;
        return;
    }

    if (newSocket->connect(*address, port) == sf::Socket::Status::Done)
    {
        std::cout << "[CLIENT] Conectado al rival " << ip << ":" << port << std::endl;
        newSocket->setBlocking(false);
        m_gameConnections.push_back(std::move(newSocket));
    }
    else
    {
        std::cerr << "[CLIENT] Error al conectar con el rival " << ip << ":" << port << std::endl;
    }
}

void NetworkManager::SendToAllConnections(sf::Packet& packet)
{
    for (auto& sock : m_gameConnections)
    {
        if (sock->send(packet) != sf::Socket::Status::Done)
        {
            std::cerr << "[CLIENT] Error al enviar paquete P2P." << std::endl;
        }
    }
}

const std::vector<std::unique_ptr<sf::TcpSocket>>& NetworkManager::GetConnections() const
{
    return m_gameConnections;
}

std::vector<std::unique_ptr<sf::TcpSocket>>& NetworkManager::GetConnections()
{
    return m_gameConnections;
}

void NetworkManager::ClearConnections()
{
    for (auto& sock : m_gameConnections)
    {
        sock->disconnect();
    }
    m_gameConnections.clear();

    // Liberar el puerto entre partidas
    if (listener != nullptr)
    {
        listener->close();
        delete listener;
        listener = nullptr;
    }

    if (m_udpSocketReady)
    {
        m_udpSocket.unbind();
        m_udpSocketReady = false;
    }

    std::cout << "[CLIENT] Conexiones P2P y listener cerrados." << std::endl;
}

bool NetworkManager::SendUdpHelloReady()
{
    auto ip = sf::IpAddress::resolve(m_clientState.gameServerIp);
    sf::IpAddress gameServerIp = ip.value();

    if (!m_udpSocketReady)
    {
        // puerto libre
        m_udpSocket.bind(sf::Socket::AnyPort);
        m_udpSocket.setBlocking(false);
        m_udpSocketReady = true;
    }

    UdpHelloData helloData;
    helloData.roomId = m_clientState.currentRoomId;
    helloData.playerId = m_clientState.playerId;

    sf::Packet packet;
    packet << PacketType::UDP_HELLO << helloData;

    m_udpSocket.send(packet, gameServerIp, m_clientState.gameServerUdpPort);

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
    m_socket.send(packet);
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

void NetworkManager::NotifyPlayerWin(const std::string& username)
{
    sf::Packet packet;
    packet << static_cast<short>(PacketType::ENDGAME);
    m_socket.send(packet);
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
        
        // Ranking y login
        if (m_clientState.hasPendingResult)
        {
            sf::Packet rankPacket;
            rankPacket << static_cast<short>(PacketType::ENDGAME);
            rankPacket << m_clientState.pendingGameResult;
            SendToServer(rankPacket);
            
            std::cout << "[CLIENT] Ranking pendiente enviado tas auto-login." << std::endl;
            m_clientState.hasPendingResult = false;
        }
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
