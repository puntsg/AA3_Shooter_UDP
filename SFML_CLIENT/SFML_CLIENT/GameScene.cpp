#include "GameScene.h"
#include "NetworkManager.h"
#include "SceneManager.h"
#include "SpriteRenderer.h"
#include "Constants.h"
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

static std::string GetPlayerNameById(int playerId)
{
    const ClientState& state = NM.GetClientState();

    // Buscamos el nombre usando el id que llega del Game Server.
    for (int i = 0; i < (int)state.roomPlayers.size(); i++)
    {
        if (state.roomPlayers[i].playerId == playerId)
            return state.roomPlayers[i].username;
    }

    return "";
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

    tileMap = new TileMap();
    tileMap->initMap("Tilemaps/Tilemap1.txt");

    m_sendTimer     = 0.f;
    m_packetSeqId   = 0;
    m_gameOverTimer = 0.f;
    m_tauntLock     = 0.f;
    m_tauntCooldown = 0.f;
    m_tauntTextTime = 0.f;
    m_tauntText.clear();
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
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
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
    if (m_tauntTextTime > 0.f)
        m_tauntTextTime -= dt;
    if (m_localHitFlash > 0.f) 
        m_localHitFlash -= dt;
    if (m_rivalHitFlash > 0.f)
        m_rivalHitFlash -= dt;

    if (cs.hasTaunt)
    {
        if (cs.tauntPlayerId != cs.playerId || m_tauntTextTime <= 0.f)
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
        }
    }
    cs.incomingTransforms.clear();

    // Spawn bullet from rival when server notifies a shot
    if (cs.hasShootReplicate)
    {
        sf::Vector2f dir = cs.lastShootReplicate.flipped
            ? sf::Vector2f(-1.f, 0.f)
            : sf::Vector2f( 1.f, 0.f);
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

        sf::Packet shootPacket;
        shootPacket << PacketType::SHOOT << shotData;
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

        
        Player* hitted = localPlayer;
        if (bullets[i]->isLocal)
            hitted = remotePlayer; 
        if (hitted && hitted->animRenderer && hitted->animRenderer->sprite.has_value())
        {
            if (bullets[i]->spriteRenderer->sprite->getGlobalBounds().findIntersection(hitted->animRenderer->sprite->getGlobalBounds()))
                bullets[i]->active = false;
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

void GameScene::DrawHitOverlay(sf::RenderWindow& window, Player* p, float flashTimer, bool rivalTint)
{

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

void GameScene::SendTaunt()
{
    if (m_tauntCooldown > 0.f)
        return;

    m_tauntCooldown = TAUNT_COOLDOWN;
    PlayTaunt(NM.GetClientState().playerId);

    sf::Packet packet;
    packet << PacketType::PLAYER_TAUNT;
    NM.SendUdp(packet);
}

void GameScene::PlayTaunt(int taunterId)
{
    ClientState& cs = NM.GetClientState();
    bool mine = taunterId == cs.playerId;

    m_tauntText = mine ? "Te estas burlando!" : "El rival se esta burlando!";
    m_tauntTextTime = TAUNT_TEXT_TIME;

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
        m_rivalHitFlash = HIT_FLASH_TIME;
    }
    else
    {
        m_rivalHealth = data.newHealth;
        m_rivalLifes = data.newLifes;
        m_rivalHitFlash = HIT_FLASH_TIME;
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
        localHud.setPosition({ 20.f, 18.f });
        window.draw(localHud);

        sf::Text rivalHud(m_font, "Rival HP:" + std::to_string(m_rivalHealth) + " Vidas:" + std::to_string(m_rivalLifes), 16);
        rivalHud.setFillColor(sf::Color::Red);
        rivalHud.setPosition({ 520.f, 18.f });
        window.draw(rivalHud);
    }

    if (m_tauntTextTime > 0.f && m_fontLoaded)
    {
        sf::Text text(m_font, m_tauntText, 24);
        text.setFillColor(sf::Color::Yellow);
        text.setPosition({ 240.f, 80.f });
        window.draw(text);
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

    NM.ClearConnections();
}

void GameScene::HandleGameEnd()
{
    // Antes de volver al lobby preparamos el resultado para el ranking.
    PrepareRankingResult();

    if (NM.ConnectToServer())
    {
        ClientState& state = NM.GetClientState();

        // Solo el ganador tiene resultado pendiente, asi no sumamos dos veces.
        if (state.hasPendingResult)
        {
            sf::Packet resultPacket;
            resultPacket << static_cast<short>(PacketType::ENDGAME);
            resultPacket << state.pendingGameResult;
            NM.SendToServer(resultPacket);
            state.hasPendingResult = false;
        }

        NM.SendLoginRequest(
            state.nickname,
            state.savedPassword
        );
    }
    SM.SetNextScene("LobbyScene");
}

void GameScene::PrepareRankingResult()
{
    ClientState& state = NM.GetClientState();
    EndgameData endData = state.endgameData;

    // Si este cliente no ha ganado no envia nada al servidor
    if (endData.winnerPlayerId != state.playerId)
        return;

    std::string winnerName = GetPlayerNameById(endData.winnerPlayerId);
    std::string loserName = GetPlayerNameById(endData.loserPlayerId);

    if (winnerName.empty() || loserName.empty())
    {
        std::cout << "[GameScene] No se pudo preparar ranking: falta nombre de jugador." << std::endl;
        return;
    }

    // Puntos del ranking: ganador suma, perdedor pierde un poco
    Result winnerResult;
    winnerResult.username = winnerName;
    winnerResult.scoredPoints = 20;

    Result loserResult;
    loserResult.username = loserName;
    loserResult.scoredPoints = -5;

    state.pendingGameResult.results.clear();
    state.pendingGameResult.results.push_back(winnerResult);
    state.pendingGameResult.results.push_back(loserResult);
    state.hasPendingResult = true;

    std::cout << "[GameScene] Resultado preparado: " << winnerName << " gana." << std::endl;
}
