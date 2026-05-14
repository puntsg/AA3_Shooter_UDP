#include "GameScene.h"
#include <iostream>
#include "../Entities/SpriteRenderer.h"

void GameScene::OnEnter()
{
    std::cout << "Entrando a GameScene..." << std::endl;
    p = new Player();
    p->GetTransform()->position = sf::Vector2f(48, 48);
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


    AnimatedRenderer* playerSprite = dynamic_cast<AnimatedRenderer*>(p->GetRenderer());
    if (!playerSprite)
        return;    
    playerSprite->sprite->setPosition(p->GetTransform()->position);
    for (int i = 0; i < t->tileGrid.size(); i++) {
        for (int j = 0; j < t->tileGrid[i].size(); j++) {
            SpriteRenderer* tileSprite = dynamic_cast<SpriteRenderer*>(t->tileGrid[i][j]->GetRenderer());
            if (tileSprite != nullptr) {
                std::optional<sf::FloatRect> collision = playerSprite->sprite->getGlobalBounds().findIntersection(tileSprite->sprite->getGlobalBounds());
                if (collision.has_value()) {
                    std::cout << "playercollied" << std::endl;
                    if (collision->size.y < collision->size.x) {
                        if (p->velocity.y > 0) {
                            p->GetTransform()->position.y -= collision->size.y;
                            p->grounded = true;
                        }
                        else
                            p->GetTransform()->position.y += collision->size.y;
                        p->velocity.y = 0;
                    }
                    else {
                        if (p->velocity.x > 0)
                            p->GetTransform()->position.x -= collision->size.x;
                        else
                            p->GetTransform()->position.x += collision->size.x;
                        p->velocity.x = 0;
                    }
                }
            }
         }
    }
}

void GameScene::Render(sf::RenderWindow& window)
{
    t->render(window);
    p->animRenderer->render(window);
}

void GameScene::OnExit()
{
    delete p;
    p = nullptr;
    delete t;
    t = nullptr;
    std::cout << "Saliendo de GameScene..." << std::endl;
}
