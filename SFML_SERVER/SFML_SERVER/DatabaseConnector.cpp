#include "DatabaseConnector.h"
#include<iostream>

DatabaseConnector::DatabaseConnector()
	: con(nullptr)
	, driver(nullptr)
{
}

void DatabaseConnector::ConnectDatabase()
{
	try {
		driver = get_driver_instance();
		con = driver->connect(SERVER, USERNAME, PASSWORD);
		con->setSchema(DATABASE);
		std::cout << "Connection done" << std::endl;
	}
	catch (sql::SQLException e) {
		con = nullptr;
		std::cout << "Could not connect. Error message: " << e.what() << std::endl;
	}
}

void DatabaseConnector::DisconnectDatabase()
{
	if (con == nullptr)
	{
		return;
	}

	con->close();
	if (con->isClosed()) {
		std::cout << "Connection closed" << std::endl;
		delete con;
		con = nullptr;
	}
}

std::string DatabaseConnector::HashPassword(const std::string& password)
{
	SHA256 sha;
	sha.update(password);
	return sha.toString(sha.digest());
}

void DatabaseConnector::GetAllPlayers()
{
	sql::PreparedStatement* pstmt = con->prepareStatement("SELECT * FROM players");
	sql::ResultSet* res = pstmt->executeQuery();
	while (res->next())
		std::cout << "Id: " << res->getInt("Id") << " | User: " << res->getString("Username") << " | Score: " << res->getInt("Score") << std::endl;
	delete res;
	delete pstmt;
}

bool DatabaseConnector::LoginPlayer(LoginRequestData lrd)
{
	if (con == nullptr)
	{
		std::cout << "LoginPlayer error: database not connected" << std::endl;
		return false;
	}

	try{
		sql::PreparedStatement* pstmt = con->prepareStatement("CALL LoginPlayer( ?, ? )");
		pstmt->setString(1, lrd.username);
		pstmt->setString(2, HashPassword(lrd.password));
		sql::ResultSet* res = pstmt->executeQuery();
		bool savedResult = res->next();
		if(savedResult)
			std::cout << "[SERVER] " << lrd.username  << " login succeed" << std::endl;
		else
			std::cout << "[SERVER] " << lrd.username << " login failed" << std::endl;
		delete res;
		
		// Consumir resultados las stored procedure
		while(pstmt->getMoreResults()) {
			sql::ResultSet* extraRes = pstmt->getResultSet();
			if(extraRes) delete extraRes;
		}
		delete pstmt;
		
		return savedResult;
	}
	catch (sql::SQLException& e) {
		std::cout << "LoginPlayer error: " << e.what() << std::endl;
		return false;
	}
}

bool DatabaseConnector::AddPlayer(RegisterRequestData rrd)
{
	if (con == nullptr)
	{
		std::cout << "AddPlayer error: database not connected" << std::endl;
		return false;
	}

	try {
		sql::PreparedStatement* pstmt = con->prepareStatement("CALL AddPlayer( ?, ? )");
		pstmt->setString(1, rrd.username);
		pstmt->setString(2, HashPassword(rrd.password));
		pstmt->execute();
		
		while(pstmt->getMoreResults()) {
			sql::ResultSet* extraRes = pstmt->getResultSet();
			if(extraRes) delete extraRes;
		}
		delete pstmt;
		return true;
	}
	catch (sql::SQLException& e) {
		std::cout << "AddPlayer error: " << e.what() << std::endl;
		return false;
	}
}

void DatabaseConnector::UpdateScore(Result r)
{
	try {
		sql::PreparedStatement* pstmt = con->prepareStatement("CALL UpdateScore( ?, ? )");
		pstmt->setString(1, r.username);
		pstmt->setInt(2, r.scoredPoints);
		pstmt->execute();
		while (pstmt->getMoreResults()) {
			sql::ResultSet* extraRes = pstmt->getResultSet();
			if (extraRes)
				delete extraRes;
		}
		delete pstmt;
	}
	catch (sql::SQLException& e) {
		std::cout << "AddPlayer error: " << e.what() << std::endl;
	}
}

std::vector<RankingData> DatabaseConnector::GetRanking(std::string playerName)
{
	std::vector<RankingData> rankingDataEntries;

	sql::PreparedStatement* pstmt = con->prepareStatement("CALL GetRanking(?)");
	pstmt->setString(1, playerName);
	sql::ResultSet* res = pstmt->executeQuery();
	while (res->next()) {
		RankingData rd;
		rd.playerName = res->getString("Username");
		rd.score = res->getInt("Score");
		rankingDataEntries.push_back(rd);
	}
	delete res;
	while (pstmt->getMoreResults()) {
		sql::ResultSet* extraRes = pstmt->getResultSet();
		if (extraRes) delete extraRes;
	}
	delete pstmt;

	return rankingDataEntries;
}

void DatabaseConnector::UpdatePlayerScore(int playerId, int scoreDiff)
{
	try {
        // En lugar de usar la stored procedure UpdateScore (que tiene el bug de Id = Id), 
        // hacemos la query pura para asegurarnos del correcto update en DB.
		sql::PreparedStatement* pstmt = con->prepareStatement(
			"UPDATE players SET Score = GREATEST(0, CAST(Score AS SIGNED) + ?) WHERE Id = ?"
		);
		pstmt->setInt(1, scoreDiff);
		pstmt->setInt(2, playerId);
		pstmt->execute();
		
		std::cout << "[SERVER] BD: Player " << playerId << " score actualizado con " << scoreDiff << " puntos." << std::endl;
        delete pstmt;
	}
	catch (sql::SQLException& e) {
		std::cout << "[SERVER] UpdatePlayerScore error: " << e.what() << std::endl;
	}
}
