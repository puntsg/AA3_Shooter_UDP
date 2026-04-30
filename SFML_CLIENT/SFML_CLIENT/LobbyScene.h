#pragma once
#include "Scene.h"
#include "NetworkManager.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <memory>
#include "SceneManager.h"
#include "Constants.h"
#include "InputField.h"
#include "Button.h"

class LobbyScene : public Scene
{
private:
    sf::Font font;

	std::unique_ptr<InputField> roomIdInput;
	std::unique_ptr<Button> createButton;
	std::unique_ptr<Button> joinButton;
    std::unique_ptr<Button> rankingButton;

    std::string statusText = "Escribe ID de la sala";
	bool uiInitialized = false;

	void BuildUI();
	void AskJoinRoom();

public:
    LobbyScene();
	
    void OnEnter() override;
    void HandleEvent(const sf::Event& event) override;
    void Update(float dt) override;
    void Render(sf::RenderWindow& window) override;
    void OnExit() override;

    void AskCreateRoom();
};
