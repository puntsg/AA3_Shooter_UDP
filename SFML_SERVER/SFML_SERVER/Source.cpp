#include <iostream>
#include <SFML/System.hpp>
#include "NetworkManager.h"
#include "DatabaseConnector.h"
constexpr unsigned short SERVER_PORT = 55000;

int main()
{
    NetworkManager networkManager;
   
    DC.ConnectDatabase();
    //DC.GetAllPlayers();
    if (!networkManager.Start(SERVER_PORT))
    {
        return -1;
    }

    while (true)
    {
        // Thread principal: acepta clientes y recibe paquetes.
        networkManager.Update();
        sf::sleep(sf::milliseconds(1));
    }
    DC.DisconnectDatabase();
    return 0;
}
