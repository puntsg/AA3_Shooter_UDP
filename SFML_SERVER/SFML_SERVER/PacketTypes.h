#pragma once
#include <SFML/Network.hpp>
enum PacketType
{
	NONE = 0,
	//Conexion inicial
	HANDSHAKE,

	//Login/registro      
	LOGIN,
	LOGIN_REQUEST,
	LOGIN_RESPONSE,

	REGISTER,
	REGISTER_REQUEST,
	REGISTER_RESPONSE,


	//Lobby
	LOBBYCREATION,
	CREATE_ROOM_REQUEST,
	CREATE_ROOM_RESPONSE,

	JOIN_ROOM_REQUEST,
	JOIN_ROOM_RESPONSE,

	//Juego
	ROOM_STATUS_UPDATE,
	START_GAME,
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

	//Generico
	ERROR_MESSAGE
};

inline sf::Packet& operator <<(sf::Packet& packet, PacketType& type)
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