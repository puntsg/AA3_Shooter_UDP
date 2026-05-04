#pragma once
#include <unordered_map>
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Scene.h"


#define SM SceneManager::Instance()

class SceneManager
{
private:
	std::unordered_map<std::string, Scene*> scenes;
	Scene* currentScene = nullptr;
	std::string nextScene = "";
	
public:
	sf::RenderWindow window;
	inline static SceneManager& Instance() {
		static SceneManager sm;
		return sm;
	}

	bool AddScene(std::string name, Scene* scene);
	Scene* GetScene(std::string name);

	bool InitFirstScene(std::string name);

	inline Scene* GetCurrentScene() const { return currentScene; }

	bool SetNextScene(std::string name);

	void UpdateCurrentScene(float dt);

private:
	SceneManager() = default;
	SceneManager(SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&);
	~SceneManager() = default;

};

