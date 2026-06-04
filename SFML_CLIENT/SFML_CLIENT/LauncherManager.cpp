#include "LauncherManager.h"
#include "PacketTypes.h"
#include "ProtocolData.h"
#include "Constants.h"
#include <iostream>
#include <fstream>
#include <filesystem>

const char* LauncherManager::MAP_PATH = "maps/map.txt";
const char* LauncherManager::VERSION_PATH = "maps/map_version.txt";
const char* LauncherManager::DEFAULT_VERSION = "0.0.0";

void LauncherManager::RunLauncher()
{
    std::cout << "[LAUNCHER] Verificando version del mapa..." << std::endl;

    sf::TcpSocket socket;
    socket.setBlocking(true);

    const char* ips[] = {
        Config::Connection::SERVER_IP,
        Config::Connection::SERVER_IP_LAN
    };

    bool connected = false;
    for (int i = 0; i < 2; i++)
    {
        std::optional<sf::IpAddress> serverIp = sf::IpAddress::resolve(ips[i]);
        if (!serverIp.has_value())
        {
            continue;
        }

        if (socket.connect(*serverIp, Config::Connection::SERVER_PORT, sf::milliseconds(1200)) == sf::Socket::Status::Done)
        {
            std::cout << "[LAUNCHER] Conectado al servidor " << ips[i] << std::endl;
            connected = true;
            break;
        }

        socket.disconnect();
    }

    if (!connected)
    {
        std::cout << "[LAUNCHER] No se pudo conectar al servidor. Continuando con mapa local." << std::endl;
        return;
    }

    DoMapHandshake(socket);
    socket.disconnect();
}

void LauncherManager::DoMapHandshake(sf::TcpSocket& socket)
{
    std::string localVersion = ReadLocalVersion();
    std::cout << "[LAUNCHER] Version local: " << localVersion << std::endl;

    // Enviar version al servidor
    MapCheckData checkData;
    checkData.version = localVersion;
    sf::Packet checkPacket;
    checkPacket << PacketType::CHECK_MAP << checkData;

    if (socket.send(checkPacket) != sf::Socket::Status::Done)
    {
        std::cout << "[LAUNCHER] Error enviando version al servidor." << std::endl;
        return;
    }

    // Recibir respuesta de estado
    sf::Packet statusPacket;
    if (socket.receive(statusPacket) != sf::Socket::Status::Done)
    {
        std::cout << "[LAUNCHER] Error recibiendo respuesta del servidor." << std::endl;
        return;
    }

    PacketType type = NONE;
    statusPacket >> type;

    if (type != MAP_STATUS)
    {
        std::cout << "[LAUNCHER] Respuesta inesperada del servidor (tipo: " << type << ")." << std::endl;
        return;
    }

    MapStatusData statusData;
    statusPacket >> statusData;

    if (statusData.upToDate)
    {
        std::cout << "[LAUNCHER] Mapa actualizado. Iniciando juego..." << std::endl;
        return;
    }

    // Mapa desactualizado: solicitar nueva version
    std::cout << "[LAUNCHER] Mapa desactualizado. Descargando nueva version..." << std::endl;

    sf::Packet requestPacket;
    requestPacket << PacketType::MAP_REQUEST;

    if (socket.send(requestPacket) != sf::Socket::Status::Done)
    {
        std::cout << "[LAUNCHER] Error solicitando mapa al servidor." << std::endl;
        return;
    }

    // Recibir contenido del mapa
    sf::Packet mapPacket;
    if (socket.receive(mapPacket) != sf::Socket::Status::Done)
    {
        std::cout << "[LAUNCHER] Error recibiendo el mapa del servidor." << std::endl;
        return;
    }

    PacketType type2 = NONE;
    mapPacket >> type2;

    if (type2 != MAP_RESPONSE)
    {
        std::cout << "[LAUNCHER] Respuesta inesperada al pedir mapa (tipo: " << type2 << ")." << std::endl;
        return;
    }

    MapResponseData mapData;
    mapPacket >> mapData;

    SaveMap(mapData.mapContent, mapData.version);
    std::cout << "[LAUNCHER] Mapa actualizado a version " << mapData.version << ". Iniciando juego..." << std::endl;
}

std::string LauncherManager::ReadLocalVersion()
{
    std::ifstream versionFile(VERSION_PATH);
    if (!versionFile.is_open())
        return DEFAULT_VERSION;

    std::string version;
    std::getline(versionFile, version);
    return version.empty() ? DEFAULT_VERSION : version;
}

void LauncherManager::SaveMap(const std::string& content, const std::string& version)
{
    std::filesystem::create_directories("maps");

    // Guardar mapa con nombre del fichero que manda el servidor
    std::string mapPath = std::string("maps/") + version;
    std::ofstream mapFile(mapPath);
    mapFile << content;

    // Guardar nombre del fichero como version local para prox comprobacion
    std::ofstream versionFile(VERSION_PATH);
    versionFile << version;
}
