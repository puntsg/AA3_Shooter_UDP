#pragma once
#include <string>
#include <SFML/Network.hpp>


class LauncherManager
{
public:
    // Conecta al servidor, comprueba la version mapa y update si es necesario
    static void RunLauncher();

private:
    static void DoMapHandshake(sf::TcpSocket& socket);

    static std::string ReadLocalVersion();

    static void SaveMap(const std::string& content, const std::string& version);

    static const char* MAP_PATH;
    static const char* VERSION_PATH;
    static const char* DEFAULT_VERSION;
};
