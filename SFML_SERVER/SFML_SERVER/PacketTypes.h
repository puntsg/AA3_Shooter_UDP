#pragma once
#include <SFML/Network.hpp>
enum PacketType
{
	NONE = 0,

	//Conexion inicial / launcher
	CHECK_MAP,
	MAP_STATUS,
	MAP_REQUEST,
	MAP_RESPONSE,

	//Login/registro      
	LOGIN_REQUEST,
	LOGIN_RESPONSE,

	REGISTER_REQUEST,
	REGISTER_RESPONSE,


	//Lobby
	CREATE_ROOM_REQUEST,
	CREATE_ROOM_RESPONSE,

	JOIN_ROOM_REQUEST,
	JOIN_ROOM_RESPONSE,

	//Juego
	ROOM_STATUS_UPDATE,
	START_GAME,
	TRANSFORM,
	SHOOT,
	SHOOT_REPLICATE,
	PLAYER_HIT,
	PLAYER_TAUNT,

	ENDGAME,
	RANKING_REQUEST,
	RANKING_RESPONSE,
	
	//Desconextion
	DISCONNECT,
	PLAYER_DISCONNECTED,

	//Generico
	ERROR_MESSAGE,

	//Bootstrap to Game Server
	SESSION_START_REQUEST = 100,
	SESSION_START_RESPONSE = 101,

	// UDP
	UDP_HELLO = 102
};

inline sf::Packet& operator <<(sf::Packet& packet, const PacketType& type)
{
	return packet << static_cast<short>(type);
}

inline sf::Packet& operator >>(sf::Packet& packet, PacketType& type)
{
	short packetType;
	packet >> packetType;
	type = static_cast<PacketType>(packetType);
	return packet;
}
