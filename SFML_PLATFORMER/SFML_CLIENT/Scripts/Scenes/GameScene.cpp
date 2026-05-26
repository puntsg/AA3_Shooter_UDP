#include "GameScene.h"
#include <iostream>
#include "../Entities/SpriteRenderer.h"
#include "../Network/NetworkManager.h"

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
    NM.NetworkFetch();

    std::vector<sf::Packet> peerPackets = NM.ReceivePeerPackets();
    for (int i = 0; i < (int)peerPackets.size(); i++)
    {
        PacketType pt = PacketType::NONE;
        peerPackets[i] >> pt;
        if (pt == PacketType::PLAYER_MOVES)
        {
            PlayerData data;
            peerPackets[i] >> data;
            std::cout << "[P2P] " << data.username << " pos(" << data.position.x << "," << data.position.y << ")" << std::endl;
        }
    }

    if (p->pendingBullet != nullptr) {
        bullets.push_back(p->pendingBullet);
        p->pendingBullet = nullptr;
    }

    for (int i = 0; i < bullets.size(); i++) {
        bullets[i]->Update(dt);
        bullets[i]->spriteRenderer->sprite->setPosition(bullets[i]->GetTransform()->position);

        for (int x = 0; x < t->tileGrid.size(); x++) {
            for (int y = 0; y < t->tileGrid[x].size(); y++) {
                if (t->tileGrid[x][y]->hasCollision) {
                    SpriteRenderer* tileSprite = dynamic_cast<SpriteRenderer*>(t->tileGrid[x][y]->GetRenderer());
                    if (tileSprite != nullptr && tileSprite->sprite.has_value()) {
                        if (bullets[i]->spriteRenderer->sprite->getGlobalBounds()
                                .findIntersection(tileSprite->sprite->getGlobalBounds()))
                            bullets[i]->active = false;
                    }
                }
            }
        }
    }

    for (int i = bullets.size() - 1; i >= 0; i--) {
        if (!bullets[i]->active) {
            delete bullets[i];
            bullets.erase(bullets.begin() + i);
        }
    }


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
        SendPlayerData();
    }
}

void GameScene::Render(sf::RenderWindow& window)
{
    t->render(window);
    p->animRenderer->render(window);
    for (int i = 0; i < (int)bullets.size(); i++)
        bullets[i]->spriteRenderer->render(window);
}

void GameScene::OnExit()
{
    delete p;
    p = nullptr;
    delete t;
    t = nullptr;
    for (int i = 0; i < (int)bullets.size(); i++)
        delete bullets[i];
    bullets.clear();
    std::cout << "Saliendo de GameScene..." << std::endl;
}

void GameScene::SendPlayerData()
{
    PlayerData data;
    data.playerId = NM.GetClientState().playerId;
    data.username = NM.GetClientState().nickname;
    data.position = p->GetTransform()->position;
    data.flipped  = p->animRenderer->flipped;
    data.health   = 0;
    data.lifes    = 0;

    sf::Packet packet;
    packet << static_cast<short>(PacketType::PLAYER_MOVES) << data;
    NM.SendToServer(packet);
    NM.SendToAllConnections(packet);
}
