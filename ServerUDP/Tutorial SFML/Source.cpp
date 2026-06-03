#include "GameServer.h"
#include <iostream>

int main()
{
	std::cout << "servidor de partidas" << std::endl;

	GameServer server;

	if (server.Start())
	{
		std::cout << " Enter cerrar ." << std::endl;
		std::cin.get();
	}
	else
	{
		std::cerr << "Fallo iniciar el servidor." << std::endl;
		std::cin.get();
	}

	server.Stop();
	return 0;
}
