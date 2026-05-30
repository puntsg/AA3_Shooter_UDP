#pragma once
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include "PacketTypes.h"
#include "ProtocolData.h"
#include <cppconn/resultset.h>
#include <sstream>
#include <iomanip>
#include "SHA256.h"


#define LISTENER_PORT 55000
#define SERVER "tcp://127.0.0.1:3306"
#define USERNAME "root"
#define PASSWORD "enti"
#define DATABASE "videogame"



#define DC DatabaseConnector::Instance()

class DatabaseConnector
{
private:
	sql::Connection* con;
	sql::Driver* driver;
	std::string HashPassword(const std::string& password);

public:
	DatabaseConnector();
	DatabaseConnector(const DatabaseConnector&) = delete;
	DatabaseConnector& operator=(const DatabaseConnector&) = delete;
	inline static DatabaseConnector& Instance()
	{
		static DatabaseConnector nm;
		return nm;
	}
	void ConnectDatabase();
	void DisconnectDatabase();

	//Get database data funcs
	void GetAllPlayers();
	bool LoginPlayer(LoginRequestData lrd);
	bool AddPlayer(RegisterRequestData rrd);
	void UpdateScore(Result r);
	void UpdatePlayerScore(int playerId, int scoreDiff);
	std::vector<RankingData> GetRanking(std::string playerName, bool& success);
};

