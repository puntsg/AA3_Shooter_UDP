#include "GameScene.h"
#include "NetworkManager.h"
#include "SceneManager.h"
#include "SpriteRenderer.h"
#include <iostream>

static const sf::Vector2f P1_START_POS(160.f, 240.f);
static const sf::Vector2f P2_START_POS(320.f, 240.f);

static int GetMyMatchIndex()
{
    const ClientState& state = NM.GetClientState();

    for (int i = 0; i < (int)state.roomPlayers.size(); i++)
    {
        if (state.roomPlayers[i].playerId == state.playerId)
            return i;
    }

    return state.isHost ? 0 : 1;
}

void GameScene::OnEnter()
{
    std::cout << "[GameScene] Iniciando partida platformer..." << std::endl;


    localPlayer = new Player();
    localPlayer->isLocal = true;
    remotePlayer = new Player();

    int myIdx = GetMyMatchIndex();
    if (myIdx == 0)
    {
        localPlayer->GetTransform()->position = P1_START_POS;
        remotePlayer->GetTransform()->position = P2_START_POS;
    }
    else
    {
        localPlayer->GetTransform()->position = P2_START_POS;
        remotePlayer->GetTransform()->position = P1_START_POS;
    }

    tileMap = new TileMap();
    tileMap->initMap("Tilemaps/Tilemap1.txt");

    m_sendTimer     = 0.f;
    m_packetSeqId   = 0;
    m_gameOverTimer = 0.f;

    NM.SendUdpHelloReady();
}

void GameScene::HandleEvent(const sf::Event& event)
{
    // Input handled inside Player::Update()
}

void GameScene::Update(float dt)
{
    NM.ReceiveUdpData();

    ClientState& cs = NM.GetClientState();

    // Apply remote player transforms from GameServer
    for (const TransformData& t : cs.incomingTransforms)
    {
        if (t.dbId != cs.playerId)
        {
            remotePlayer->GetTransform()->position = sf::Vector2f(t.x, t.y);
            remotePlayer->animRenderer->flipped    = t.flipped;
        }
    }
    cs.incomingTransforms.clear();

    // Spawn bullet from rival when server notifies a shot
    if (cs.hasShootReplicate)
    {
        sf::Vector2f dir = cs.lastShootReplicate.flipped
            ? sf::Vector2f(-1.f, 0.f)
            : sf::Vector2f( 1.f, 0.f);
        bullets.push_back(new Bullet(cs.lastShootReplicate.position, dir));
        cs.hasShootReplicate = false;
    }

    // Endgame countdown
    if (cs.hasEndgame)
    {
        cs.hasEndgame   = false;
        m_gameOverTimer = 3.f;
    }

    if (m_gameOverTimer > 0.f)
    {
        m_gameOverTimer -= dt;
        if (m_gameOverTimer <= 0.f)
            HandleGameEnd();
        return;
    }

    // Update local player (input + physics)
    localPlayer->Update(dt);

    // Forward any bullet the player just spawned
    if (localPlayer->pendingBullet != nullptr)
    {
        bullets.push_back(localPlayer->pendingBullet);
        localPlayer->pendingBullet = nullptr;

        sf::Packet shootPacket;
        shootPacket << PacketType::SHOOT;
        NM.SendUdp(shootPacket);
    }

    // Update & cull bullets
    for (int i = 0; i < (int)bullets.size(); i++)
    {
        bullets[i]->Update(dt);
        bullets[i]->spriteRenderer->sprite->setPosition(bullets[i]->GetTransform()->position);

        for (auto& row : tileMap->tileGrid)
        {
            for (Tile* tile : row)
            {
                if (!tile->hasCollision) continue;
                SpriteRenderer* ts = dynamic_cast<SpriteRenderer*>(tile->GetRenderer());
                if (!ts || !ts->sprite.has_value()) continue;
                if (bullets[i]->spriteRenderer->sprite->getGlobalBounds()
                        .findIntersection(ts->sprite->getGlobalBounds()))
                    bullets[i]->active = false;
            }
        }
    }

    for (int i = (int)bullets.size() - 1; i >= 0; i--)
    {
        if (!bullets[i]->active)
        {
            delete bullets[i];
            bullets.erase(bullets.begin() + i);
        }
    }

    // Collision between local player and tilemap
    ResolveCollisions(localPlayer);

    // Keep remote player animation ticking
    remotePlayer->animRenderer->Update(dt);

    // Send transform to GameServer periodically
    m_sendTimer += dt;
    if (m_sendTimer >= SEND_INTERVAL)
    {
        SendTransform();
        m_sendTimer = 0.f;
    }
}

void GameScene::ResolveCollisions(Player* p)
{
    AnimatedRenderer* ar = p->animRenderer;
    if (!ar || !ar->sprite.has_value()) return;

    for (auto& row : tileMap->tileGrid)
    {
        for (Tile* tile : row)
        {
            if (!tile->hasCollision) continue;
            SpriteRenderer* ts = dynamic_cast<SpriteRenderer*>(tile->GetRenderer());
            if (!ts || !ts->sprite.has_value()) continue;

            auto collision = ar->sprite->getGlobalBounds()
                .findIntersection(ts->sprite->getGlobalBounds());
            if (!collision.has_value()) continue;

            if (collision->size.y < collision->size.x)
            {
                if (p->velocity.y > 0)
                {
                    p->GetTransform()->position.y -= collision->size.y;
                    p->grounded = true;
                }
                else
                    p->GetTransform()->position.y += collision->size.y;
                p->velocity.y = 0.f;
            }
            else
            {
                if (p->velocity.x > 0)
                    p->GetTransform()->position.x -= collision->size.x;
                else
                    p->GetTransform()->position.x += collision->size.x;
                p->velocity.x = 0.f;
            }

            ar->sprite->setPosition(p->GetTransform()->position);
        }
    }
}

void GameScene::SendTransform()
{
    TransformData data;
    data.packetId      = ++m_packetSeqId;
    data.dbId          = NM.GetClientState().playerId;
    data.localPlayerId = -1;
    data.x             = localPlayer->GetTransform()->position.x;
    data.y             = localPlayer->GetTransform()->position.y;
    data.flipped       = localPlayer->animRenderer->flipped;

    sf::Packet packet;
    packet << PacketType::TRANSFORM << data;
    NM.SendUdp(packet);
}

void GameScene::Render(sf::RenderWindow& window)
{
    tileMap->render(window);

    for (Bullet* b : bullets)
        b->spriteRenderer->render(window);

    remotePlayer->animRenderer->render(window);
    localPlayer->animRenderer->render(window);
}

void GameScene::OnExit()
{
    std::cout << "[GameScene] Saliendo de la partida." << std::endl;

    sf::Packet packet;
    packet << PacketType::DISCONNECT;
    NM.SendUdp(packet);

    delete localPlayer;  localPlayer  = nullptr;
    delete remotePlayer; remotePlayer = nullptr;
    delete tileMap;      tileMap      = nullptr;

    for (Bullet* b : bullets) delete b;
    bullets.clear();

    NM.ClearConnections();
}

void GameScene::HandleGameEnd()
{
    if (NM.ConnectToServer())
    {
        NM.SendLoginRequest(
            NM.GetClientState().nickname,
            NM.GetClientState().savedPassword
        );
    }
    SM.SetNextScene("LobbyScene");
}
