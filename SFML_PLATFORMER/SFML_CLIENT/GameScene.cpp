#include "GameScene.h"
#include <iostream>


void GameScene::OnEnter()
{
    std::cout << "Entrando a GameScene..." << std::endl;
 }

void GameScene::HandleEvent(const sf::Event& event) {
    if (event.is<sf::Event::MouseButtonPressed>())
    {
        const sf::Event::MouseButtonPressed* mbInfo =
            event.getIf<sf::Event::MouseButtonPressed>();

        //if (mbInfo && mbInfo->button == sf::Mouse::Button::Left)
    }
}

void GameScene::Update(float dt)
{
    
}

void GameScene::Render(sf::RenderWindow& window)
{
    
}

void GameScene::OnExit()
{
    std::cout << "Saliendo de GameScene..." << std::endl;
    
}
