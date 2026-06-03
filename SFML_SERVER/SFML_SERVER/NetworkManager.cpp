#include "NetworkManager.h"
#include "DatabaseConnector.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <SFML/System.hpp>

static constexpr const char* MAPS_DIR = "maps/";

static constexpr bool USE_LAN = false;

static constexpr const char* GAME_SERVER_LAN_IP        = "192.168.1.45";
static constexpr const char* GAME_SERVER_PUBLIC_IP_WAN = "79.152.44.136";  
static constexpr const char* GAME_SERVER_LINK_IP       = "127.0.0.1";      
static constexpr unsigned short GAME_SERVER_UDP_PORT   = 55002;

static constexpr const char* GAME_SERVER_PUBLIC_IP = USE_LAN
    ? GAME_SERVER_LAN_IP
    : GAME_SERVER_PUBLIC_IP_WAN;

// Busca txt devuelve nombre
static std::string GetCurrentMapFilename()
{
    for (const auto& entry : std::filesystem::directory_iterator(MAPS_DIR))
    {
        if (entry.path().extension() == ".txt")
            return entry.path().filename().string();
    }
    return "";
}

NetworkManager::NetworkManager()
    : m_isRunning(false)
    , m_nextPlayerId(1)
    , m_threadPool(this, 4)
{
    m_listener.setBlocking(false);
}

bool NetworkManager::Start(unsigned short listenPort)
{
    if (m_listener.listen(listenPort) != sf::Socket::Status::Done)
    {
        std::cerr << "[SERVER] Error al escuchar en puerto " << listenPort << std::endl;
        return false;
    }

    m_listener.setBlocking(false);
    m_isRunning = true;

    std::cout << "[SERVER] Escuchando en puerto " << listenPort << std::endl;
    return true;
}

void NetworkManager::Update()
{
    if (!m_isRunning)
    {
        return;
    }

    AcceptNewClients();
    ReceiveClientData();
}

void NetworkManager::AcceptNewClients()
{
    std::unique_ptr<sf::TcpSocket> newSocket = std::make_unique<sf::TcpSocket>();
    newSocket->setBlocking(false);

    if (m_listener.accept(*newSocket) == sf::Socket::Status::Done)
    {
        ConnectedClient newClient{};
        newClient.playerId = m_nextPlayerId++;
        newClient.socket = newSocket.get();
        newClient.ip = newSocket->getRemoteAddress().value_or(sf::IpAddress::Any);

        std::cout << "[SERVER] Nuevo cliente conectado. playerId: "
            << newClient.playerId
            << " | IP: "
            << newClient.ip.toString()
            << std::endl;

        std::lock_guard<std::mutex> lock(m_stateMutex);
        m_sockets.push_back(std::move(newSocket));
        m_clients.push_back(newClient);
    }
}

void NetworkManager::ReceiveClientData()
{
    for (int i = 0; i < static_cast<int>(m_clients.size()); ++i)
    {
        ConnectedClient& client = m_clients[i];

        if (client.socket == nullptr)
        {
            continue;
        }

        sf::Packet packet;
        sf::Socket::Status status = client.socket->receive(packet);

        while (status == sf::Socket::Status::Done)
        {
            // El main thread manda el paquete al thread pool.
            const int playerId = client.playerId;
            m_threadPool.Enqueue(playerId, packet);

            packet.clear();
            status = client.socket->receive(packet);
        }

        if (status == sf::Socket::Status::Disconnected)
        {
            std::cout << "[SERVER] Cliente desconectado. playerId: "
                << client.playerId
                << std::endl;

            RemoveDisconnectedClient(i);
            --i;
        }
    }
}

void NetworkManager::ProcessPacketFromPool(int playerId, sf::Packet packet)
{
    // Protegemos el estado porque varios threads pueden llegar aqui.
    std::lock_guard<std::mutex> lock(m_stateMutex);

    ConnectedClient* client = GetClientById(playerId);
    if (client == nullptr || client->socket == nullptr)
    {
        return;
    }

    ProcessPacket(*client, packet);
}

void NetworkManager::ProcessPacket(ConnectedClient& client, sf::Packet& packet)
{
    PacketType packetType = PacketType::NONE;
    packet >> packetType;
    std::cout << "[SERVER] Processing package: " << packetType << std::endl << " from: " << client.username << std::endl;
    switch (packetType)
    {
    case PacketType::CHECK_MAP:
        HandleCheckMap(client, packet);
        break;
    case PacketType::MAP_REQUEST:
        HandleMapRequest(client);
        break;
    case PacketType::REGISTER_REQUEST:
        HandleRegisterRequest(client, packet);
        break;
    case PacketType::LOGIN_REQUEST:
        HandleLoginRequest(client, packet);
        break;
    case PacketType::CREATE_ROOM_REQUEST:
        HandleCreateRoomRequest(client, packet);
        break;
    case PacketType::JOIN_ROOM_REQUEST:
        HandleJoinRoomRequest(client, packet);
        break;
    case PacketType::RANKING_REQUEST:
        HandleRankingRequest(client, packet);
        break;
    case PacketType::ENDGAME:
        HandleEndGame(client, packet);
        break;
    case PacketType::DISCONNECT:
        HandleDisconnectRequest(client);
        break;
    case PacketType::PLAYER_MOVES:
        std::cout << "player send movement packet" << std::endl;
        break;
    default:
        std::cout << "[SERVER] Paquete no gestionado recibido de playerId "
            << client.playerId
            << std::endl;
        break;
    }
}

void NetworkManager::HandleCheckMap(ConnectedClient& client, sf::Packet& packet)
{
    MapCheckData checkData;
    packet >> checkData;

    std::string currentFilename = GetCurrentMapFilename();

    MapStatusData statusData;
    statusData.upToDate = (!currentFilename.empty() && checkData.version == currentFilename);

    sf::Packet responsePacket;
    responsePacket << PacketType::MAP_STATUS << statusData;
    client.socket->send(responsePacket);

    std::cout << "[SERVER] CheckMap de playerId " << client.playerId
              << " | cliente: " << checkData.version
              << " | servidor: " << currentFilename
              << " | upToDate: " << statusData.upToDate << std::endl;
}

void NetworkManager::HandleMapRequest(ConnectedClient& client)
{
    std::string currentFilename = GetCurrentMapFilename();
    std::string mapFilePath     = std::string(MAPS_DIR) + currentFilename;

    std::ifstream mapFile(mapFilePath);
    std::string mapContent;

    if (mapFile.is_open())
    {
        mapContent = std::string(std::istreambuf_iterator<char>(mapFile),
                                 std::istreambuf_iterator<char>());
    }
    else
    {
        std::cerr << "[SERVER] No se encontro el archivo de mapa: " << mapFilePath << std::endl;
    }

    MapResponseData mapData;
    mapData.version    = currentFilename;   // nombre del .txt
    mapData.mapContent = mapContent;

    sf::Packet responsePacket;
    responsePacket << PacketType::MAP_RESPONSE << mapData;
    client.socket->send(responsePacket);

    std::cout << "[SERVER] Mapa '" << currentFilename << "' enviado a playerId " << client.playerId << std::endl;
}

void NetworkManager::HandleRegisterRequest(ConnectedClient& client, sf::Packet& packet)
{
    RegisterRequestData registerRequestData;
    packet >> registerRequestData;

    RegisterResponseData response;
    response.success = DC.AddPlayer(registerRequestData);
    response.message = response.success ? "Registro completado." : "No se pudo registrar. El usuario puede existir.";
    SendRegisterResponse(client, response);
}

void NetworkManager::HandleLoginRequest(ConnectedClient& client, sf::Packet& packet)
{
    LoginRequestData loginRequestData;
    packet >> loginRequestData;

    bool alreadyConnected = std::find(
        connectedUsers.begin(),
        connectedUsers.end(),
        loginRequestData.username) != connectedUsers.end();
    bool success = DC.LoginPlayer(loginRequestData);
    LoginResponseData response;
    response.success = !alreadyConnected && success;
    if (alreadyConnected)
    {
        response.message = "Usuario ya conectado.";
    }
    else if (response.success) {
        client.username = loginRequestData.username;
        response.username = loginRequestData.username;
        response.playerId = client.playerId;
        connectedUsers.push_back(loginRequestData.username);
        response.message = "Login correcto.";
    }
    else
    {
        response.message = "Login incorrecto.";
    }
    SendLoginResponse(client, response);
}

void NetworkManager::HandleCreateRoomRequest(ConnectedClient& client, sf::Packet& packet)
{
    CreateRoomRequestData requestData;
    packet >> requestData;

    if (requestData.roomId == "__queue_normal")
    {
        HandleMatchmakingRequest(client, requestData, false);
        return;
    }

    if (requestData.roomId == "__queue_ranked")
    {
        HandleMatchmakingRequest(client, requestData, true);
        return;
    }

    client.username = requestData.username;
    client.gamePort = requestData.gamePort;

    bool success = m_roomManager.CreateRoom(requestData.roomId, client.playerId);

    if (!success)
    {
        SendCreateRoomResponse(client, false, requestData.roomId, "La sala ya existe.");
        return;
    }

    client.currentRoomId = requestData.roomId;

    SendCreateRoomResponse(client, true, requestData.roomId, "Sala creada correctamente.");
    BroadcastRoomStatus(requestData.roomId);
    TryStartGame(requestData.roomId);

    PrintConnectedClients();
    m_roomManager.PrintRooms();
}

void NetworkManager::HandleMatchmakingRequest(ConnectedClient& client, const CreateRoomRequestData& requestData, bool ranked)
{
    client.username = requestData.username;
    client.gamePort = requestData.gamePort;

    std::vector<int>& queue = ranked ? m_rankedQueue : m_normalQueue;
    const std::string queueName = ranked ? "ranked" : "normal";

    RemoveClientFromMatchmakingQueues(client.playerId);

    queue.push_back(client.playerId);
    client.currentRoomId = "__queue_" + queueName;

    SendCreateRoomResponse(client, true, client.currentRoomId, "Buscando partida " + queueName + ".");

    std::cout << "[SERVER][Matchmaking] Player " << client.playerId
        << " en cola " << queueName
        << " | " << queue.size() << "/2"
        << std::endl;

    TryCreateMatchFromQueue(queue, queueName);
}

void NetworkManager::HandleJoinRoomRequest(ConnectedClient& client, sf::Packet& packet)
{
    JoinRoomRequestData requestData;
    packet >> requestData;

    client.username = requestData.username;
    client.gamePort = requestData.gamePort;

    Room* room = m_roomManager.GetRoom(requestData.roomId);

    if (room == nullptr)
    {
        SendJoinRoomResponse(client, false, requestData.roomId, "La sala no existe.");
        return;
    }

    if (room->inGame)
    {
        SendJoinRoomResponse(client, false, requestData.roomId, "La sala ya esta en partida.");
        return;
    }

    if (static_cast<int>(room->playerIds.size()) >= room->maxPlayers)
    {
        SendJoinRoomResponse(client, false, requestData.roomId, "La sala esta llena.");
        return;
    }

    bool success = m_roomManager.JoinRoom(requestData.roomId, client.playerId);

    if (!success)
    {
        SendJoinRoomResponse(client, false, requestData.roomId, "No se ha podido unir a la sala.");
        return;
    }

    client.currentRoomId = requestData.roomId;

    SendJoinRoomResponse(client, true, requestData.roomId, "Te has unido a la sala correctamente.");
    BroadcastRoomStatus(requestData.roomId);
    TryStartGame(requestData.roomId);
    PrintConnectedClients();
    m_roomManager.PrintRooms();
}

void NetworkManager::HandleEndGame(ConnectedClient& client, sf::Packet& packet)
{
    GameResultData resultData;
    packet >> resultData;
    for (const Result& r : resultData.results)
        DC.UpdateScore(r);
    
    // Eliminar la sala para volver a jugar si quieren
    Room* room = m_roomManager.GetRoomByPlayerId(client.playerId);
    if (room != nullptr)
    {
        std::string roomId = room->roomId;
        m_roomManager.DeleteRoom(roomId);
        std::cout << "[SERVER] Sala " << roomId << " eliminada tras ENDGAME." << std::endl;
    }
}

void NetworkManager::HandleRankingRequest(ConnectedClient& client, sf::Packet& packet)
{
    RankingRequestData rankingRequestData;
    packet >> rankingRequestData;

    bool rankingOk = false;
    std::vector<RankingData> dbRanking = DC.GetRanking(rankingRequestData.username, rankingOk);

    RankingResponseData response;
    response.success = rankingOk;
    response.message = rankingOk ? "Ranking cargado." : "No se pudo cargar el ranking.";
    if (rankingOk && dbRanking.empty())
    {
        response.message = "No hay datos de ranking.";
    }

    for (int i = 0; i < dbRanking.size(); i++)
        response.entries.push_back(dbRanking[i]);

    sf::Packet responsePacket;
    responsePacket << static_cast<short>(PacketType::RANKING_RESPONSE);
    responsePacket << response;
    client.socket->send(responsePacket);
}

void NetworkManager::HandleDisconnectRequest(ConnectedClient& client)
{
    RemoveClientFromMatchmakingQueues(client.playerId);

    if (client.currentRoomId.rfind("__queue_", 0) == 0)
    {
        client.currentRoomId.clear();
    }

    SendCreateRoomResponse(client, false, "", "Busqueda cancelada.");
    std::cout << "[SERVER][Matchmaking] Busqueda cancelada para playerId "
        << client.playerId
        << std::endl;
}



void NetworkManager::SendCreateRoomResponse(ConnectedClient& client, bool success, const std::string& roomId, const std::string& message)
{
    if (client.socket == nullptr)
    {
        return;
    }

    sf::Packet packet;
    CreateRoomResponseData responseData;
    responseData.success = success;
    responseData.roomId = roomId;
    responseData.message = message;

    packet << static_cast<short>(PacketType::CREATE_ROOM_RESPONSE);
    packet << responseData;

    client.socket->send(packet);
}

void NetworkManager::SendJoinRoomResponse(ConnectedClient& client, bool success, const std::string& roomId, const std::string& message)
{
    if (client.socket == nullptr)
    {
        return;
    }

    sf::Packet packet;
    JoinRoomResponseData responseData;
    responseData.success = success;
    responseData.roomId = roomId;
    responseData.message = message;

    packet << static_cast<short>(PacketType::JOIN_ROOM_RESPONSE);
    packet << responseData;

    client.socket->send(packet);
}

void NetworkManager::SendLoginResponse(ConnectedClient& client, const LoginResponseData& data)
{
    sf::Packet packet;
    packet << static_cast<short>(PacketType::LOGIN_RESPONSE);
    packet << data;
    client.socket->send(packet);
}

void NetworkManager::SendRegisterResponse(ConnectedClient& client, const RegisterResponseData& data)
{
    sf::Packet packet;
    packet << static_cast<short>(PacketType::REGISTER_RESPONSE);
    packet << data;
    client.socket->send(packet);
}



void NetworkManager::SendErrorMessage(ConnectedClient& client, const std::string& message)
{
    if (client.socket == nullptr)
    {
        return;
    }

    sf::Packet packet;
    ErrorMessageData errorData;
    errorData.message = message;

    packet << static_cast<short>(PacketType::ERROR_MESSAGE);
    packet << errorData;

    client.socket->send(packet);
}

bool NetworkManager::SendSessionToGameServer(const StartGameData& startData, std::string& message)
{
    auto gameServerIp = sf::IpAddress::resolve(GAME_SERVER_LINK_IP);
    if (!gameServerIp.has_value())
    {
        message = "IP del Game Server invalida.";
        return false;
    }

    sf::UdpSocket gameServerSocket;
    if (gameServerSocket.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done)
    {
        message = "No se pudo abrir UDP para hablar con el Game Server.";
        return false;
    }
    gameServerSocket.setBlocking(false);

    SessionStartData sessionData;
    sessionData.roomId = startData.roomId;
    sessionData.playerCount = startData.playerCount;
    sessionData.players = startData.players;

    // sala para el udp
    sf::Packet requestPacket;
    requestPacket << static_cast<short>(PacketType::SESSION_START_REQUEST);
    requestPacket << sessionData;

    if (gameServerSocket.send(requestPacket, *gameServerIp, GAME_SERVER_UDP_PORT) != sf::Socket::Status::Done)
    {
        message = "No se pudo enviar la sesion al Game Server.";
        gameServerSocket.unbind();
        return false;
    }

    sf::Clock waitClock;

    // espera corta
    while (waitClock.getElapsedTime().asMilliseconds() < 1500)
    {
        sf::Packet responsePacket;
        std::optional<sf::IpAddress> senderIp;
        unsigned short senderPort = 0;
        sf::Socket::Status status = gameServerSocket.receive(responsePacket, senderIp, senderPort);

        if (status == sf::Socket::Status::Done)
        {
            PacketType responseType = PacketType::NONE;
            responsePacket >> responseType;

            if (responseType != PacketType::SESSION_START_RESPONSE)
                continue;

            SessionStartResponseData responseData;
            responsePacket >> responseData;
            message = responseData.message;
            gameServerSocket.unbind();
            return responseData.success;
        }

        sf::sleep(sf::milliseconds(10));
    }

    message = "El Game Server no respondio a tiempo.";
    gameServerSocket.unbind();
    return false;
}

void NetworkManager::BroadcastRoomStatus(const std::string& roomId)
{
    Room* room = m_roomManager.GetRoom(roomId);

    if (room == nullptr)
    {
        return;
    }

    RoomStatusUpdateData roomData;
    roomData.roomId = room->roomId;
    roomData.currentPlayers = static_cast<short>(room->playerIds.size());
    roomData.maxPlayers = room->maxPlayers;

    roomData.players.clear();

    for (int playerId : room->playerIds)
    {
        ConnectedClient* roomClient = GetClientById(playerId);
        if (roomClient == nullptr)
        {
            continue;
        }

        LobbyPlayerInfo playerInfo;
        playerInfo.playerId = roomClient->playerId;
        playerInfo.username = roomClient->username;
        playerInfo.ip = roomClient->ip.toString();
        playerInfo.gamePort = roomClient->gamePort;
        playerInfo.isHost = (playerId == room->playerIds.front());

        roomData.players.push_back(playerInfo);
    }

    for (int playerId : room->playerIds)
    {
        ConnectedClient* roomClient = GetClientById(playerId);
        if (roomClient == nullptr || roomClient->socket == nullptr)
        {
            continue;
        }

        sf::Packet packet;
        packet << static_cast<short>(PacketType::ROOM_STATUS_UPDATE);
        packet << roomData;
        roomClient->socket->send(packet);
    }
}

void NetworkManager::TryStartGame(const std::string& roomId)
{
    Room* room = m_roomManager.GetRoom(roomId);

    if (room == nullptr)
    {
        return;
    }

    if (static_cast<short>(room->playerIds.size()) < room->maxPlayers)
    {
        return;
    }

    room->inGame = true;

    StartGameData startData;
    startData.roomId = room->roomId;
    startData.playerCount = static_cast<short>(room->playerIds.size());
    startData.gameServerIp = GAME_SERVER_PUBLIC_IP;
    startData.gameServerUdpPort = GAME_SERVER_UDP_PORT;

    for (int playerId : room->playerIds)
    {
        ConnectedClient* roomClient = GetClientById(playerId);
        if (roomClient == nullptr)
        {
            continue;
        }

        LobbyPlayerInfo playerInfo;
        playerInfo.playerId = roomClient->playerId;
        playerInfo.username = roomClient->username;
        playerInfo.ip = roomClient->ip.toString();
        playerInfo.gamePort = roomClient->gamePort;
        playerInfo.isHost = (playerId == room->playerIds.front());

        startData.players.push_back(playerInfo);
    }

    std::string gameServerMessage;
    // antes de mandar START_GAME
    if (!SendSessionToGameServer(startData, gameServerMessage))
    {
        room->inGame = false;

        std::cerr << "[SERVER] No se pudo iniciar Game Server para sala "
            << roomId
            << ": "
            << gameServerMessage
            << std::endl;

        for (int playerId : room->playerIds)
        {
            ConnectedClient* roomClient = GetClientById(playerId);
            if (roomClient != nullptr)
            {
                SendErrorMessage(*roomClient, "No se pudo iniciar la partida: " + gameServerMessage);
            }
        }

        return;
    }

    std::cout << "[SERVER] Game Server preparado para sala "
        << roomId
        << ": "
        << gameServerMessage
        << std::endl;

    for (int playerId : room->playerIds)
    {
        ConnectedClient* roomClient = GetClientById(playerId);
        if (roomClient == nullptr || roomClient->socket == nullptr)
        {
            continue;
        }

        sf::Packet packet;
        packet << static_cast<short>(PacketType::START_GAME);
        packet << startData;
        roomClient->socket->send(packet);
    }

    std::cout << "[SERVER] START_GAME enviado para sala " << roomId << std::endl;

    for (int playerId : room->playerIds)
    {
        ConnectedClient* roomClient = GetClientById(playerId);
        if (roomClient != nullptr)
        {
            roomClient->currentRoomId.clear();
        }
    }

    m_roomManager.DeleteRoom(roomId);
}

void NetworkManager::TryCreateMatchFromQueue(std::vector<int>& queue, const std::string& queueName)
{
    while (queue.size() >= 2)
    {
        const int firstPlayerId = queue.front();
        queue.erase(queue.begin());
        const int secondPlayerId = queue.front();
        queue.erase(queue.begin());

        ConnectedClient* firstClient = GetClientById(firstPlayerId);
        ConnectedClient* secondClient = GetClientById(secondPlayerId);

        if (firstClient == nullptr || secondClient == nullptr)
        {
            continue;
        }

        long long now = std::chrono::steady_clock::now().time_since_epoch().count();
        std::string roomId = "match_" + queueName + "_" + std::to_string(firstPlayerId) + "_" + std::to_string(secondPlayerId) + "_" + std::to_string(now);

        if (!m_roomManager.CreateRoom(roomId, firstPlayerId))
        {
            SendErrorMessage(*firstClient, "No se pudo crear la partida.");
            SendErrorMessage(*secondClient, "No se pudo crear la partida.");
            continue;
        }

        if (!m_roomManager.JoinRoom(roomId, secondPlayerId))
        {
            m_roomManager.DeleteRoom(roomId);
            SendErrorMessage(*firstClient, "No se pudo crear la partida.");
            SendErrorMessage(*secondClient, "No se pudo crear la partida.");
            continue;
        }

        firstClient->currentRoomId = roomId;
        secondClient->currentRoomId = roomId;

        SendCreateRoomResponse(*firstClient, true, roomId, "Partida encontrada.");
        SendJoinRoomResponse(*secondClient, true, roomId, "Partida encontrada.");
        BroadcastRoomStatus(roomId);
        TryStartGame(roomId);

        std::cout << "[SERVER][Matchmaking] Match " << queueName
            << " creado: " << roomId
            << std::endl;
    }
}

void NetworkManager::RemoveClientFromMatchmakingQueues(int playerId)
{
    auto removeFromQueue = [playerId](std::vector<int>& queue)
    {
        queue.erase(std::remove(queue.begin(), queue.end(), playerId), queue.end());
    };

    removeFromQueue(m_normalQueue);
    removeFromQueue(m_rankedQueue);
}

ConnectedClient* NetworkManager::GetClientById(int playerId)
{
    for (ConnectedClient& client : m_clients)
    {
        if (client.playerId == playerId)
        {
            return &client;
        }
    }

    return nullptr;
}

ConnectedClient* NetworkManager::GetClientBySocket(sf::TcpSocket* socket)
{
    for (ConnectedClient& client : m_clients)
    {
        if (client.socket == socket)
        {
            return &client;
        }
    }

    return nullptr;
}

void NetworkManager::RemoveDisconnectedClient(int index)
{
    std::lock_guard<std::mutex> lock(m_stateMutex);

    if (index < 0 || index >= static_cast<int>(m_clients.size()))
    {
        return;
    }

    int playerId = m_clients[index].playerId;

    if (!connectedUsers.empty()) {
        auto it = std::find(connectedUsers.begin(), connectedUsers.end(), m_clients[index].username);
        if (it != connectedUsers.end())
            connectedUsers.erase(it);
    }
    RemoveClientFromMatchmakingQueues(playerId);
    m_roomManager.RemovePlayerFromRoom(playerId);

    if (index < static_cast<int>(m_sockets.size()))
    {
        if (m_sockets[index] != nullptr)
        {
            m_sockets[index]->disconnect();
        }

        m_sockets.erase(m_sockets.begin() + index);
    }

    m_clients.erase(m_clients.begin() + index);
    PrintConnectedClients();
    m_roomManager.PrintRooms();
}

void NetworkManager::PrintConnectedClients() const
{
    std::cout << "\n[SERVER] Clientes conectados:\n";

    if (m_clients.empty())
    {
        std::cout << "  No hay clientes conectados.\n";
        return;
    }

    for (const ConnectedClient& client : m_clients)
    {
        std::cout << "  playerId: " << client.playerId
            << " | username: " << client.username
            << " | roomId: " << client.currentRoomId
            << " | ip: " << client.ip.toString()
            << " | gamePort: " << client.gamePort
            << "\n";
    }
}

void NetworkManager::HandleRankingUpdate(ConnectedClient& client, sf::Packet& packet)
{
    RankingUpdateData updateData;
    packet >> updateData;

    std::cout << "[SERVER] Recibida actualización de ranking de sala " << updateData.roomId 
              << " por jugador " << client.playerId << std::endl;

    pendingRankingUpdates[updateData.roomId].push_back(updateData);
    ProcessRankingValidation(updateData.roomId);
}

void NetworkManager::ProcessRankingValidation(const std::string& roomId)
{
    auto& updates = pendingRankingUpdates[roomId];
    if (updates.size() < 2) return; // 2 Updates iguales para validar

    // Verificación por pares
    bool same = true;
    const auto& first = updates[0].placementOrder;
    for (size_t i = 1; i < updates.size(); ++i)
    {
        if (updates[i].placementOrder != first)
        {
            same = false;
            break;
        }
    }

    if (same)
    {
        std::cout << "[SERVER] Ranking validado para sala " << roomId << ". Actualizando BD..." << std::endl;
        
        for (size_t i = 0; i < first.size(); ++i)
        {
            int playerId = first[i];
            int pointsDiff = 0;
            if (i == 0) pointsDiff = 20;       // Ganador
            else if (i == 1) pointsDiff = -5;  // 2do lugar
            else pointsDiff = -10;             // 3er y 4to lugar

            if (pointsDiff != 0) {
                DC.UpdatePlayerScore(playerId, pointsDiff);
            }
        }

        
        pendingRankingUpdates.erase(roomId);
    }
    else if (updates.size() >= 4) 
    {
        std::cout << "[SERVER] Discrepancia insalvable en ranking de sala " << roomId << ". Anulando." << std::endl;
        pendingRankingUpdates.erase(roomId);
    }
}

