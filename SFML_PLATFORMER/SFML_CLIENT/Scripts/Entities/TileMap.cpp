#include "TileMap.h"
#include <iostream>
#include <fstream>

void TileMap::initMap(std::string mapDatafilePath)
{
	//https://www.w3schools.com/cpp/cpp_files.asp
	sf::Vector2 currentPos = sf::Vector2(0, 0);
	std::string fileString;
	std::ifstream MyReadFile(mapDatafilePath);
	std::cout << "Generating map";
	while (std::getline(MyReadFile,fileString)) {
		std::cout << fileString + "\n";
		//aqui inicialzar las tiles
	}
	MyReadFile.close();
}
