#pragma once
#include "Scene.h"
#include <iostream>
#include <functional>
#include <SFML/Graphics.hpp>
#include "SceneManager.h"
#include <string>
#include "../Entities/UI/Button.h"
#include "../Constants.h"

class TitleScene: public Scene
{
private:
    sf::Font font;
    Button* playButton;

public:
    TitleScene();

    void OnEnter() override;

    void HandleEvent(const sf::Event& event) override;

    void Update(float dt) override;

    void Render(sf::RenderWindow& window) override;

    void OnExit() override;

};

