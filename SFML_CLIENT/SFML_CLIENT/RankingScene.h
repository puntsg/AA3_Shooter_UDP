#pragma once
#include "Scene.h"
#include "NetworkManager.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include "SceneManager.h"
#include "Button.h"

class RankingScene : public Scene
{
private:
    sf::Font font;
    std::unique_ptr<Button> backButton;
    std::vector<std::pair<std::string, int>> rankingData;

    void BuildUI();

public:
    RankingScene();

    void OnEnter() override;
    void HandleEvent(const sf::Event& event) override;
    void Update(float dt) override;
    void Render(sf::RenderWindow& window) override;
    void OnExit() override;
};