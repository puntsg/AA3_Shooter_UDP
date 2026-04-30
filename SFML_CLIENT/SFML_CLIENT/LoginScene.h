#pragma once
#include "Scene.h"
#include "NetworkManager.h"
#include <iostream>
#include <functional>
#include <SFML/Graphics.hpp>
#include "SceneManager.h"
#include <string>
#include "Button.h"
#include "InputField.h"
#include "Constants.h"

class LoginScene: public Scene
{
private:
    sf::Font font;
    InputField* usernameInputfield;
    InputField* passwordInputfield;
    Button* loginButton;
    Button* signinButton;
    Button* closeButton;

public:
    LoginScene();

    void OnEnter() override;

    void HandleEvent(const sf::Event& event) override;

    void Update(float dt) override;

    void Render(sf::RenderWindow& window) override;

    void OnExit() override;

};

