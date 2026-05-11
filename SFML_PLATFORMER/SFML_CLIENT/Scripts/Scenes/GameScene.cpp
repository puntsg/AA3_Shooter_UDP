#include "GameScene.h"
#include <iostream>

void GameScene::OnEnter()
{
    std::cout << "Entrando a GameScene..." << std::endl;
    p = new Player();
    t = new TileMap();
    t->initMap("Tilemaps/Tilemap1.txt");
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
    p->Update(dt);
}

void GameScene::Render(sf::RenderWindow& window)
{
    p->animRenderer->render(window);
}

void GameScene::OnExit()
{
    delete p;
    p = nullptr;
    std::cout << "Saliendo de GameScene..." << std::endl;
    
}
