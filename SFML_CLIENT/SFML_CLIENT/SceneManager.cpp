#include "SceneManager.h"

bool SceneManager::AddScene(std::string name, Scene* scene) {
	
	if (scenes.find(name) == scenes.end()) {
		scenes.emplace(name, scene);
		return true;
	}
	return false;
}

bool SceneManager::InitFirstScene(std::string name) {

	if (scenes.find(name) != scenes.end()) {
		currentScene = scenes[name];
		currentScene->OnEnter();
		return true;
	}
	return false;
}

bool SceneManager::SetNextScene(std::string name) {
	if (scenes.find(name) == scenes.end()) {
		return false;
	}
	nextScene = name;
	return true;
}

void SceneManager::UpdateCurrentScene(float dt) {
	if (nextScene != "") {
		if (currentScene)
			currentScene->OnExit();
		std::cout << "Exiting from:  " << currentScene << ", netxt scene: " << scenes[nextScene] << std::endl;
		currentScene = scenes[nextScene];
		if (currentScene)
			currentScene->OnEnter();
		nextScene = "";
	}
	if (currentScene) currentScene->Update(dt);
}
