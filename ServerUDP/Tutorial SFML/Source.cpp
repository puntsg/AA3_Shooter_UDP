#include <SFML/Network.hpp>
#include <iostream>
#include <string>

#define BIND_PORT 55000
//SERVER
void main()
{
	sf::UdpSocket socket;

	if (socket.bind(BIND_PORT) == sf::Socket::Status::Done)
	{
		std::cout << "Puerto binded a okay" << std::endl;
	}
	else
	{
		std::cerr << " Hizo Pum... ya esta aqui la guerra" << std::endl;
	}
	int receivedCount = 0;
	int receivedLenght = 0;
	char buffer[1024];
	while (true)
	{
		std::size_t receivedData;
		std::size_t receivedLength;
		std::size_t receivedLengthCount;
		std::optional <sf::IpAddress> senderIP;
		unsigned short senderPort;

		if (socket.receive(buffer, sizeof(buffer), receivedData, senderIP, senderPort) == sf::Socket::Status::Done)
		{
			std::cout << "Mensaje recibido de " << senderIP.value() << ": " << senderPort << std::endl;
			
			std::size_t byteRead = 0;

			int messageSize = 0;
			std::memcpy(&messageSize, buffer, sizeof(messageSize));

			byteRead += sizeof(messageSize);

			std::string receivedString(buffer + byteRead, messageSize);
			byteRead += messageSize;

			int receivedData;
			std::memcpy(&receivedData, buffer + byteRead, sizeof(receivedData));
			
			


			std::cout << receivedString << " :: " << receivedData << std::endl;
		}
	}

	
}