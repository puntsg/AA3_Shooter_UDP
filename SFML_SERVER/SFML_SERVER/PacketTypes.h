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

	// Conecta3 antiguo
	PIECEADDED,
	NEXT_TURN,

	ENDGAME,
	RANKINGUPDATE,
	RANKING_REQUEST,
	RANKING_RESPONSE,
	
	//Player
	PLAYER_MOVES,
	PLAYER_SHOTS,
	PLAYER_HURTED,
	BULLET_MOVES,

	//Desconextion
	DISCONNECT,
	PLAYER_DISCONNECTED,
	PING,

	//Generico
	ERROR_MESSAGE,

	//Server TCP to Game Server
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
