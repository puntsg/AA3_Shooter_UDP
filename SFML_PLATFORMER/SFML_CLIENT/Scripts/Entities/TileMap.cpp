#include "TileMap.h"
#include <iostream>
#include <fstream>

void TileMap::initMap(std::string mapDatafilePath)
{
	//https://www.w3schools.com/cpp/cpp_files.asp
	std::string fileString;
	std::ifstream MyReadFile(mapDatafilePath);
	std::cout << "Generating map";
	while (std::getline(MyReadFile,fileString)) {
		std::cout << fileString;
		//aqui inicialzar las tiles
	}
	MyReadFile.close();
}
