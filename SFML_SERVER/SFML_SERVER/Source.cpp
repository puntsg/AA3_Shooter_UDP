#include <iostream>
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
        networkManager.Update();
    }
    DC.DisconnectDatabase();
    return 0;
}