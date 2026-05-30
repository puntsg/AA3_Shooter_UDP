#include "GameScene.h"
#include "NetworkManager.h"
#include "SceneManager.h"
#include <iostream>

void GameScene::OnEnter()
{
    std::cout << "[GameScene] Iniciando partida shooter..." << std::endl;

    const ClientState& state = NM.GetClientState();
    m_gameManager.InitGame(state.roomPlayers, state.playerId);
    m_gameOverTimer = 0.f;

    // Registrar endpoint
    NM.SendUdpHelloReady();
}

void GameScene::HandleEvent(const sf::Event& event)
{
    m_gameManager.HandleInput(event);
}

void GameScene::Update(float dt)
{
    NM.ReceiveUdpData();

    ClientState& cs = NM.GetClientState();

    for (const TransformData& t : cs.incomingTransforms)
        m_gameManager.ApplyTransform(t);

    if (cs.hasShootReplicate)
    {
        m_gameManager.SpawnRivalBullet(cs.lastShootReplicate);
        cs.hasShootReplicate = false;
    }

    if (cs.hasPlayerHit)
    {
        m_gameManager.ApplyHit(cs.lastPlayerHit);
        cs.hasPlayerHit = false;
    }

    if (cs.hasTaunt)
    {
        m_gameManager.ApplyTaunt(cs.tauntPlayerId);
        cs.hasTaunt = false;
    }

    if (cs.hasEndgame)
    {
        m_gameManager.ApplyEndgame(cs.endgameData);
        cs.hasEndgame = false;
    }

    m_gameManager.Update(dt);

    if (m_gameManager.IsGameOver())
    {
        m_gameOverTimer += dt;
        if (m_gameOverTimer >= 3.f)
            HandleGameEnd();
    }
}

void GameScene::Render(sf::RenderWindow& window)
{
    m_gameManager.DrawGame(window);
    m_gameManager.DrawHUD(window);
}

void GameScene::OnExit()
{
    std::cout << "[GameScene] Saliendo de la partida." << std::endl;
    NM.ClearConnections();
    m_gameManager.Reset();
}

void GameScene::HandleGameEnd()
{
    // Reconectar al servidor bootstrap para volver al lobby
    if (NM.ConnectToServer())
    {
        NM.SendLoginRequest(
            NM.GetClientState().nickname,
            NM.GetClientState().savedPassword
        );
        std::cout << "[GameScene] Reconectado al servidor bootstrap." << std::endl;
    }
    else
    {
        std::cerr << "[GameScene] No se pudo reconectar al servidor." << std::endl;
    }

    SM.SetNextScene("LobbyScene");
}
