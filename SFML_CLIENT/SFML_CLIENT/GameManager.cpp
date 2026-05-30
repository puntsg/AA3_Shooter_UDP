#include "GameManager.h"
#include "NetworkManager.h"
#include "Constants.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

GameManager::GameManager() {}

void GameManager::InitGame(const std::vector<LobbyPlayerInfo>& players, int localDbId)
{
    m_localDbId = localDbId;
    m_myLocalId = -1;

    // Config jugador local azul y rival rojo
    m_local = PlayerVisual();
    m_rival = PlayerVisual();

    for (const LobbyPlayerInfo& p : players)
    {
        if (p.playerId == localDbId)
        {
            m_local.dbId     = p.playerId;
            m_local.nickname = p.username;
            m_local.shape.setSize({ PLAYER_W, PLAYER_H });
            m_local.shape.setFillColor(sf::Color::Cyan);
        }
        else
        {
            m_rival.dbId     = p.playerId;
            m_rival.nickname = p.username;
            m_rival.shape.setSize({ PLAYER_W, PLAYER_H });
            m_rival.shape.setFillColor(sf::Color::Red);
        }
    }

    // Pos iniciales
    m_local.position = { 300.f, 200.f };
    m_rival.position = { 500.f, 200.f };
    m_local.health = MAX_HEALTH;   m_local.lifes = MAX_LIFES;
    m_rival.health = MAX_HEALTH;   m_rival.lifes = MAX_LIFES;

    LoadMap();

    if (!m_font.openFromFile(Config::Assets::FONT_PATH))
        m_font.openFromFile(Config::Assets::FONT_PATH_FALLBACK);

    m_soundLoaded = m_tauntBuffer.loadFromFile(TAUNT_SOUND);
    if (m_soundLoaded)
        m_tauntSound.setBuffer(m_tauntBuffer);

    m_gameOver  = false;
    m_localWon  = false;
    m_sendTimer = 0.f;
    m_packetSeqId = 0;
    m_bullets.clear();
    std::cout << "[GameManager] Partida iniciada. Local: " << m_local.nickname
              << " vs " << m_rival.nickname << std::endl;
}

void GameManager::LoadMap()
{
    std::string mapPath = FALLBACK_MAP;
    std::ifstream versionFile(VERSION_FILE);
    if (versionFile.is_open())
    {
        std::string filename;
        std::getline(versionFile, filename);
        if (!filename.empty())
            mapPath = "maps/" + filename;
    }

    std::ifstream mapFile(mapPath);
    if (!mapFile.is_open())
    {
        std::cerr << "[GameManager] No se pudo abrir el mapa: " << mapPath << std::endl;
        return;
    }

    m_mapRows.clear();
    m_tiles.clear();

    std::string row;
    int rowIdx = 0;
    while (std::getline(mapFile, row))
    {
        m_mapRows.push_back(row);
        for (int col = 0; col < static_cast<int>(row.size()); col++)
        {
            if (row[col] == '#')
            {
                sf::RectangleShape tile({ TILE_SIZE, TILE_SIZE });
                tile.setPosition({ col * TILE_SIZE, MAP_OFFSET_Y + rowIdx * TILE_SIZE });
                tile.setFillColor(sf::Color(70, 70, 70));
                tile.setOutlineColor(sf::Color(110, 110, 110));
                tile.setOutlineThickness(1.f);
                m_tiles.push_back(tile);
            }
        }
        rowIdx++;
    }

    m_mapColCount = m_mapRows.empty() ? 0 : static_cast<int>(m_mapRows[0].size());
    m_mapRowCount = static_cast<int>(m_mapRows.size());
    std::cout << "[GameManager] Mapa cargado: " << mapPath
              << " (" << m_mapColCount << "x" << m_mapRowCount << ")" << std::endl;
}


void GameManager::HandleInput(const sf::Event& event)
{
    if (m_gameOver) return;

    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        switch (key->code)
        {
        case sf::Keyboard::Key::Left:
        case sf::Keyboard::Key::A:
            m_leftHeld = true;
            break;
        case sf::Keyboard::Key::Right:
        case sf::Keyboard::Key::D:
            m_rightHeld = true;
            break;
        case sf::Keyboard::Key::Space:
        case sf::Keyboard::Key::W:
        case sf::Keyboard::Key::Up:
            if (m_local.onGround)
            {
                m_local.velocity.y = JUMP_FORCE;
                m_local.onGround   = false;
            }
            break;
        case sf::Keyboard::Key::F:
            SendShoot();
            break;
        case sf::Keyboard::Key::T:
            SendTaunt();
            break;
        default:
            break;
        }
    }

    if (const auto* key = event.getIf<sf::Event::KeyReleased>())
    {
        switch (key->code)
        {
        case sf::Keyboard::Key::Left:
        case sf::Keyboard::Key::A:
            m_leftHeld = false;
            break;
        case sf::Keyboard::Key::Right:
        case sf::Keyboard::Key::D:
            m_rightHeld = false;
            break;
        default:
            break;
        }
    }
}


void GameManager::Update(float dt)
{
    if (m_gameOver) return;

    // Movimiento horizontal local
    if (m_leftHeld)
    {
        m_local.velocity.x = -PLAYER_SPEED;
        m_local.flipped    = true;
    }
    else if (m_rightHeld)
    {
        m_local.velocity.x = PLAYER_SPEED;
        m_local.flipped    = false;
    }
    else
    {
        m_local.velocity.x = 0.f;
    }

    // Fisica del jugador local, prediccion cliente
    ApplyPhysics(m_local, dt);

    UpdateBullets(dt);

    if (m_showTaunt)
    {
        m_tauntTimer -= dt;
        if (m_tauntTimer <= 0.f)
            m_showTaunt = false;
    }

    // Enviar transform al servidor cada x tiempo
    m_sendTimer += dt;
    if (m_sendTimer >= SEND_INTERVAL)
    {
        SendTransform();
        m_sendTimer = 0.f;
    }
}

void GameManager::ApplyPhysics(PlayerVisual& player, float dt)
{
    player.velocity.y += GRAVITY * dt;
    player.position   += player.velocity * dt;
    player.onGround    = false;
    ResolveCollisions(player);
}

void GameManager::ResolveCollisions(PlayerVisual& player)
{
    sf::FloatRect pRect(player.position, { PLAYER_W, PLAYER_H });

    for (int row = 0; row < m_mapRowCount; row++)
    {
        for (int col = 0; col < m_mapColCount; col++)
        {
            if (row >= static_cast<int>(m_mapRows.size()))  continue;
            if (col >= static_cast<int>(m_mapRows[row].size())) continue;
            if (m_mapRows[row][col] != '#') continue;

            sf::FloatRect tRect(
                { col * TILE_SIZE, row * TILE_SIZE },
                { TILE_SIZE, TILE_SIZE }
            );

            if (!pRect.findIntersection(tRect)) continue;

            float overlapL = (pRect.position.x + PLAYER_W) - tRect.position.x;
            float overlapR = (tRect.position.x + TILE_SIZE) - pRect.position.x;
            float overlapT = (pRect.position.y + PLAYER_H) - tRect.position.y;
            float overlapB = (tRect.position.y + TILE_SIZE) - pRect.position.y;

            float minH = std::min(overlapL, overlapR);
            float minV = std::min(overlapT, overlapB);

            if (minV <= minH)
            {
                if (overlapT < overlapB)
                {
                    player.position.y -= overlapT;
                    player.velocity.y  = 0.f;
                    player.onGround    = true;
                }
                else
                {
                    player.position.y += overlapB;
                    player.velocity.y  = 0.f;
                }
            }
            else
            {
                if (overlapL < overlapR)
                    player.position.x -= overlapL;
                else
                    player.position.x += overlapR;
                player.velocity.x = 0.f;
            }

            // Actualizar rect tras la correccion
            pRect.position = player.position;
        }
    }
}

void GameManager::UpdateBullets(float dt)
{
    for (Bullet& b : m_bullets)
    {
        float delta     = BULLET_SPEED * dt;
        b.position.x   += b.direction * delta;
        b.traveled     += delta;

        if (b.traveled >= BULLET_MAX_DIST) { b.active = false; continue; }

        // Colision con tile del mapa 
        int col = static_cast<int>(b.position.x / TILE_SIZE);
        int row = static_cast<int>(b.position.y / TILE_SIZE);
        if (row >= 0 && row < m_mapRowCount && col >= 0 && col < m_mapColCount
            && m_mapRows[row][col] == '#')
        {
            b.active = false;
        }
    }

    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
            [](const Bullet& b) { return !b.active; }),
        m_bullets.end()
    );
}


void GameManager::SendTransform()
{
    TransformData data;
    data.packetId      = ++m_packetSeqId;
    data.dbId          = m_local.dbId;
    data.localPlayerId = m_myLocalId;
    data.x             = m_local.position.x;
    data.y             = m_local.position.y;
    data.flipped       = m_local.flipped;

    sf::Packet packet;
    packet << PacketType::TRANSFORM << data;
    NM.SendUdp(packet);
}

void GameManager::SendShoot()
{
    // Crear bala local. solo local el daño update en el server
    Bullet b;
    float bx   = m_local.flipped ? m_local.position.x : m_local.position.x + PLAYER_W;
    float by   = m_local.position.y + PLAYER_H * 0.5f;
    b.position = { bx, by };
    b.direction = m_local.flipped ? -1.f : 1.f;
    b.fromLocal = true;
    m_bullets.push_back(b);

    sf::Packet packet;
    packet << PacketType::SHOOT;
    NM.SendUdp(packet);
}

void GameManager::SendTaunt()
{
    sf::Packet packet;
    packet << PacketType::PLAYER_TAUNT;
    NM.SendUdp(packet);
}


void GameManager::ApplyTransform(const TransformData& data)
{
    if (data.dbId == m_local.dbId)
    {
        // Aprender localId propio en el primer paquete
        if (m_myLocalId == -1)
            m_myLocalId = data.localPlayerId;

        // Reconciliacion
        float dx = std::abs(data.x - m_local.position.x);
        float dy = std::abs(data.y - m_local.position.y);
        if (dx > SNAP_THRESHOLD || dy > SNAP_THRESHOLD)
        {
            m_local.position = { data.x, data.y };
            m_local.flipped  = data.flipped;
            std::cout << "[GameManager] Reconciliacion aplicada." << std::endl;
        }
    }
    else
    {
        // Actualizar pos del rival directamente
        m_rival.position = { data.x, data.y };
        m_rival.flipped  = data.flipped;
    }
}

void GameManager::ApplyHit(const PlayerHitData& data)
{
    PlayerVisual& target = (data.targetPlayerId == m_local.dbId) ? m_local : m_rival;
    target.health = data.newHealth;
    target.lifes  = data.newLifes;

    if (data.newHealth == MAX_HEALTH)
    {
        target.position = data.respawnPosition;
        target.velocity = { 0.f, 0.f };
    }
}

void GameManager::ApplyTaunt(int taunterDbId)
{
    const std::string& who = (taunterDbId == m_local.dbId)
        ? m_local.nickname : m_rival.nickname;

    m_tauntMsg   = who + " se esta burlando!";
    m_showTaunt  = true;
    m_tauntTimer = TAUNT_DURATION;

    if (m_soundLoaded)
        m_tauntSound.play();
}

void GameManager::SpawnRivalBullet(const ShootReplicateData& data)
{
    Bullet b;
    b.position  = data.position;
    b.direction = data.flipped ? -1.f : 1.f;
    b.fromLocal = false;
    m_bullets.push_back(b);
}

void GameManager::ApplyEndgame(const EndgameData& data)
{
    m_gameOver = true;
    m_localWon = (data.winnerPlayerId == m_local.dbId);
    std::cout << "[GameManager] Fin de partida. " << (m_localWon ? "Victoria!" : "Derrota.") << std::endl;
}


void GameManager::DrawGame(sf::RenderWindow& window)
{
    // Mapa
    for (const sf::RectangleShape& tile : m_tiles)
        window.draw(tile);

    // Balas
    sf::RectangleShape bulletShape({ BULLET_W, BULLET_H });
    for (const Bullet& b : m_bullets)
    {
        bulletShape.setFillColor(b.fromLocal ? sf::Color::Yellow : sf::Color(255, 150, 0));
        // Convertir pos de mundo a pantalla 
        bulletShape.setPosition({ b.position.x, b.position.y + MAP_OFFSET_Y });
        window.draw(bulletShape);
    }

    // Enemigo
    m_rival.shape.setPosition({ m_rival.position.x, m_rival.position.y + MAP_OFFSET_Y });
    window.draw(m_rival.shape);

    // Jugador local
    m_local.shape.setPosition({ m_local.position.x, m_local.position.y + MAP_OFFSET_Y });
    window.draw(m_local.shape);

    // Mensaje de taunt
    if (m_showTaunt)
    {
        sf::Text tauntText(m_font, m_tauntMsg, 26);
        tauntText.setFillColor(sf::Color::Yellow);
        tauntText.setPosition({ 200.f, 280.f });
        window.draw(tauntText);
    }

    // Pantalla de fin de partida
    if (m_gameOver)
    {
        sf::Text endText(m_font, m_localWon ? "VICTORIA!" : "DERROTA...", 60);
        endText.setFillColor(m_localWon ? sf::Color::Green : sf::Color::Red);
        endText.setPosition({ 220.f, 240.f });
        window.draw(endText);

        sf::Text subText(m_font, "Volviendo al lobby...", 22);
        subText.setFillColor(sf::Color::White);
        subText.setPosition({ 270.f, 320.f });
        window.draw(subText);
    }
}

void GameManager::DrawHUD(sf::RenderWindow& window)
{
    // HUD fondo
    sf::RectangleShape hudBg({ 800.f, MAP_OFFSET_Y - 4.f });
    hudBg.setFillColor(sf::Color(20, 20, 20, 200));
    hudBg.setPosition({ 0.f, 0.f });
    window.draw(hudBg);

    // Local (azul)
    std::string localStr = m_local.nickname + "  HP:" + std::to_string(m_local.health)
        + "  Vidas:" + std::to_string(m_local.lifes);
    sf::Text localHud(m_font, localStr, 18);
    localHud.setFillColor(sf::Color::Cyan);
    localHud.setPosition({ 10.f, 18.f });
    window.draw(localHud);

    // Rival (rojo)
    std::string rivalStr = m_rival.nickname + "  HP:" + std::to_string(m_rival.health)
        + "  Vidas:" + std::to_string(m_rival.lifes);
    sf::Text rivalHud(m_font, rivalStr, 18);
    rivalHud.setFillColor(sf::Color::Red);
    rivalHud.setPosition({ 450.f, 18.f });
    window.draw(rivalHud);

    // Controles 
    sf::Text controls(m_font, "A/D:Mover  W/Espacio:Saltar  F:Disparar  T:Taunt", 12);
    controls.setFillColor(sf::Color(150, 150, 150));
    controls.setPosition({ 180.f, 4.f });
    window.draw(controls);
}


bool GameManager::IsGameOver()          const { return m_gameOver; }
bool GameManager::IsLocalPlayerWinner() const { return m_localWon; }

void GameManager::Reset()
{
    m_local       = PlayerVisual();
    m_rival       = PlayerVisual();
    m_bullets.clear();
    m_gameOver    = false;
    m_localWon    = false;
    m_showTaunt   = false;
    m_sendTimer   = 0.f;
    m_packetSeqId = 0;
    m_myLocalId   = -1;
    m_leftHeld    = false;
    m_rightHeld   = false;
    m_mapRows.clear();
    m_tiles.clear();
}
