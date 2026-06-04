#include "GameScene.h"
#include "NetworkManager.h"
#include "SceneManager.h"
#include "SpriteRenderer.h"
#include "Constants.h"
#include <cstddef>
#include <iostream>

static const sf::Vector2f P1_START_POS(Config::Gameplay::P1_SPAWN_X, Config::Gameplay::P1_SPAWN_Y);
static const sf::Vector2f P2_START_POS(Config::Gameplay::P2_SPAWN_X, Config::Gameplay::P2_SPAWN_Y);

static int GetMyMatchIndex()
{
    const ClientState& state = NM.GetClientState();

    for (std::size_t i = 0; i < state.roomPlayers.size(); ++i)
    {
        if (state.roomPlayers[i].playerId == state.playerId)
            return static_cast<int>(i);
    }

    return state.isHost ? 0 : 1;
}

void GameScene::OnEnter()
{
    std::cout << "[GameScene] Iniciando partida platformer..." << std::endl;


    localPlayer = new Player();
    localPlayer->isLocal = true;
    remotePlayer = new Player();
    localPlayer->animRenderer->tint = sf::Color::White;
    remotePlayer->animRenderer->tint = sf::Color(255, 120, 120);

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

    remotePlayer->animRenderer->Update(0.f);

    tileMap = new TileMap();
    tileMap->initMap("Tilemaps/Tilemap1.txt");

    m_sendTimer     = 0.f;
    m_packetSeqId   = 0;
    m_gameOverTimer = 0.f;
    m_tauntLock     = 0.f;
    m_tauntCooldown = 0.f;
    m_tauntEchoBlockTime = 0.f;
    m_localHealth = MAX_HEALTH;
    m_localLifes = MAX_LIFES;
    m_rivalHealth = MAX_HEALTH;
    m_rivalLifes = MAX_LIFES;

    m_soundLoaded = m_tauntBuffer.loadFromFile(TAUNT_SOUND);
    if (m_soundLoaded)
        m_tauntSound.emplace(m_tauntBuffer);

    m_fontLoaded = m_font.openFromFile(Config::Assets::FONT_PATH);
    if (!m_fontLoaded)
        m_fontLoaded = m_font.openFromFile(Config::Assets::FONT_PATH_FALLBACK);

    NM.SendUdpHelloReady();
}

void GameScene::HandleEvent(const sf::Event& event)
{
    if (const sf::Event::KeyPressed* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::T)
            SendTaunt();
    }
}

void GameScene::Update(float dt)
{
    NM.ReceiveUdpData();

    ClientState& cs = NM.GetClientState();

    if (m_tauntCooldown > 0.f)
        m_tauntCooldown -= dt;
    if (m_tauntEchoBlockTime > 0.f)
        m_tauntEchoBlockTime -= dt;

    if (cs.hasTaunt)
    {
        if (cs.tauntPlayerId != cs.playerId || m_tauntEchoBlockTime <= 0.f)
            PlayTaunt(cs.tauntPlayerId);

        cs.hasTaunt = false;
        cs.tauntPlayerId = -1;
    }

    if (cs.hasPlayerHit)
    {
        ApplyPlayerHit(cs.lastPlayerHit);
        cs.hasPlayerHit = false;
    }

    // Apply remote player transforms from GameServer
    for (const TransformData& t : cs.incomingTransforms)
    {
        if (t.dbId != cs.playerId)
        {
            remotePlayer->GetTransform()->position = sf::Vector2f(t.x, t.y);
            remotePlayer->animRenderer->flipped    = t.flipped;
            remotePlayer->animRenderer->ApplyFrameRect(
                { t.spriteStartX, t.spriteStartY },
                { t.spriteEndX,   t.spriteEndY });
        }
    }
    cs.incomingTransforms.clear();

    // Spawn bullet from rival when server notifies a shot
    if (cs.hasShootReplicate)
    {
        sf::Vector2f dir = cs.lastShootReplicate.flipped
            ? sf::Vector2f( 1.f, 0.f)
            : sf::Vector2f(-1.f, 0.f);
        Bullet* rivalBullet = new Bullet(cs.lastShootReplicate.position, dir);
        rivalBullet->isLocal = false;
        bullets.push_back(rivalBullet);
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

    if (m_tauntLock > 0.f)
        m_tauntLock -= dt;

    localPlayer->inputLocked = m_tauntLock > 0.f;

    // Update local player (input + physics)
    localPlayer->Update(dt);

    // Forward any bullet the player just spawned
    if (localPlayer->pendingBullet != nullptr)
    {
        ShootReplicateData shotData;
        shotData.position = localPlayer->pendingBullet->GetTransform()->position;
        shotData.flipped = localPlayer->animRenderer->flipped;

        bullets.push_back(localPlayer->pendingBullet);
        localPlayer->pendingBullet = nullptr;

        NM.SendCriticalShoot(shotData);
    }

    // Update & cull bullets
    for (Bullet* bullet : bullets)
    {
        bullet->Update(dt);
        bullet->spriteRenderer->sprite->setPosition(bullet->GetTransform()->position);

        for (std::vector<Tile*>& row : tileMap->tileGrid)
        {
            for (Tile* tile : row)
            {
                if (!tile->hasCollision) continue;
                SpriteRenderer* ts = dynamic_cast<SpriteRenderer*>(tile->GetRenderer());
                if (!ts || !ts->sprite.has_value()) continue;
                if (bullet->spriteRenderer->sprite->getGlobalBounds()
                        .findIntersection(ts->sprite->getGlobalBounds()))
                    bullet->active = false;
            }
        }

        if (!bullet->active)
            continue;
        
        Player* hitted = localPlayer;
        if (bullet->isLocal)
            hitted = remotePlayer; 
        if (hitted && hitted->animRenderer && hitted->animRenderer->sprite.has_value())
        {
            if (bullet->spriteRenderer->sprite->getGlobalBounds().findIntersection(hitted->animRenderer->sprite->getGlobalBounds()))
                bullet->active = false;
        }
    }

    for (std::size_t i = bullets.size(); i-- > 0;)
    {
        if (!bullets[i]->active)
        {
            delete bullets[i];
            bullets.erase(bullets.begin() + i);
        }
    }

    ResolveCollisions(localPlayer);
    ResolvePlayerCollision();
    ClampLocalPlayerToMap();
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

    for (std::vector<Tile*>& row : tileMap->tileGrid)
    {
        for (Tile* tile : row)
        {
            if (!tile->hasCollision) continue;
            SpriteRenderer* ts = dynamic_cast<SpriteRenderer*>(tile->GetRenderer());
            if (!ts || !ts->sprite.has_value()) continue;

            std::optional<sf::FloatRect> collision = ar->sprite->getGlobalBounds()
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

void GameScene::ResolvePlayerCollision()
{
    if (!localPlayer || !remotePlayer) return;

    AnimatedRenderer* la = localPlayer->animRenderer;
    AnimatedRenderer* ra = remotePlayer->animRenderer;
    if (!la || !ra || !la->sprite.has_value() || !ra->sprite.has_value()) return;

    sf::FloatRect a = la->sprite->getGlobalBounds();
    sf::FloatRect b = ra->sprite->getGlobalBounds();

    std::optional<sf::FloatRect> inter = a.findIntersection(b);
    if (!inter.has_value()) return;

    // Empujar SOLO al jugador local, por el eje de menor penetracion (como con los tiles)
    if (inter->size.x < inter->size.y)
    {
        float aCenter = a.position.x + a.size.x * 0.5f;
        float bCenter = b.position.x + b.size.x * 0.5f;
        if (aCenter < bCenter)
            localPlayer->GetTransform()->position.x -= inter->size.x;
        else
            localPlayer->GetTransform()->position.x += inter->size.x;
        localPlayer->velocity.x = 0.f;
    }
    else
    {
        float aCenter = a.position.y + a.size.y * 0.5f;
        float bCenter = b.position.y + b.size.y * 0.5f;
        if (aCenter < bCenter)
        {
            localPlayer->GetTransform()->position.y -= inter->size.y;
            localPlayer->grounded = true;
        }
        else
            localPlayer->GetTransform()->position.y += inter->size.y;
        localPlayer->velocity.y = 0.f;
    }

    la->sprite->setPosition(localPlayer->GetTransform()->position);
}

void GameScene::ClampLocalPlayerToMap()
{
    if (!localPlayer || !tileMap || tileMap->tileGrid.empty())
        return;

    float mapBottomY = static_cast<float>(tileMap->tileGrid.size())
        * static_cast<float>(tileMap->tileSize.y);

    if (localPlayer->GetTransform()->position.y < mapBottomY)
        return;

    localPlayer->GetTransform()->position.y = mapBottomY - FALL_RESET_OFFSET;
    localPlayer->velocity.y = 0.f;
    localPlayer->grounded = true;

    if (localPlayer->animRenderer && localPlayer->animRenderer->sprite.has_value())
        localPlayer->animRenderer->sprite->setPosition(localPlayer->GetTransform()->position);
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
    data.spriteStartX = localPlayer->animRenderer->startOffset.x;
    data.spriteStartY = localPlayer->animRenderer->startOffset.y;
    data.spriteEndX = localPlayer->animRenderer->endOffset.x;
    data.spriteEndY = localPlayer->animRenderer->endOffset.y;

    UdpPacketHeaderData header;
    header.flags = PACKET_FLAG_URGENT;
    header.packetId = data.packetId;

    sf::Packet packet;
    packet << PacketType::TRANSFORM << header << data;
    NM.SendUdp(packet);
}

void GameScene::SendTaunt()
{
    if (m_tauntCooldown > 0.f)
        return;

    m_tauntCooldown = TAUNT_COOLDOWN;
    PlayTaunt(NM.GetClientState().playerId);

    NM.SendCriticalTaunt();
}

void GameScene::PlayTaunt(int taunterId)
{
    ClientState& cs = NM.GetClientState();
    bool mine = taunterId == cs.playerId;

    m_tauntEchoBlockTime = TAUNT_ECHO_BLOCK_TIME;

    if (mine)
        m_tauntLock = TAUNT_LOCK_TIME;

    if (m_soundLoaded && m_tauntSound)
        m_tauntSound->play();
}

void GameScene::ApplyPlayerHit(const PlayerHitData& data)
{
    ClientState& cs = NM.GetClientState();
    Player* target = (data.targetPlayerId == cs.playerId) ? localPlayer : remotePlayer;

    if (data.targetPlayerId == cs.playerId)
    {
        m_localHealth = data.newHealth;
        m_localLifes = data.newLifes;
    }
    else
    {
        m_rivalHealth = data.newHealth;
        m_rivalLifes = data.newLifes;
    }

    std::cout << "[CLIENT] Hit player " << data.targetPlayerId
        << " hp: " << data.newHealth
        << " vidas: " << data.newLifes
        << std::endl;

    if (target != nullptr && data.newHealth == MAX_HEALTH)
    {
        target->GetTransform()->position = data.respawnPosition;
        target->velocity = sf::Vector2f(0.f, 0.f);
        if (target->animRenderer && target->animRenderer->sprite.has_value())
            target->animRenderer->sprite->setPosition(data.respawnPosition);
    }
}

void GameScene::Render(sf::RenderWindow& window)
{
    tileMap->render(window);

    for (Bullet* b : bullets)
        b->spriteRenderer->render(window);

    remotePlayer->animRenderer->render(window);
    localPlayer->animRenderer->render(window);

    if (m_fontLoaded)
    {
        sf::Text localHud(m_font, "Tu HP:" + std::to_string(m_localHealth) + " Vidas:" + std::to_string(m_localLifes), 16);
        localHud.setFillColor(sf::Color::Cyan);
        localHud.setPosition({ 20.f, static_cast<float>(Config::Window::HEIGHT) - 36.f });
        window.draw(localHud);

        sf::Text rivalHud(m_font, "Rival HP:" + std::to_string(m_rivalHealth) + " Vidas:" + std::to_string(m_rivalLifes), 16);
        rivalHud.setFillColor(sf::Color::Red);
        sf::FloatRect rivalBounds = rivalHud.getLocalBounds();
        rivalHud.setPosition({
            static_cast<float>(Config::Window::WIDTH) - rivalBounds.position.x - rivalBounds.size.x - 20.f,
            static_cast<float>(Config::Window::HEIGHT) - 36.f
        });
        window.draw(rivalHud);
    }
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

    NM.ClearGameNetworkState();
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
