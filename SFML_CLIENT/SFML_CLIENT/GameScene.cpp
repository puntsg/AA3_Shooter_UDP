#include "GameScene.h"
#include "NetworkManager.h"
#include <iostream>

void GameScene::SetupGame(const std::vector<Player>& players, int localID)
{
    gameManager.InitGame(players, localID);
}
void GameScene::SyncNextTurn(int playerID)
{
    gameManager.SyncNextTurn(playerID);
}

void GameScene::OnEnter()
{
    std::cout << "Entrando a GameScene..." << std::endl;
    auto& state = NM.GetClientState();
    std::vector<Player> gamePlayers;
    int myIndex = -1;

    for (int i = 0; i < (int)state.roomPlayers.size(); ++i)
    {
        const auto& lp = state.roomPlayers[i];
        Player p;
        p.id = lp.playerId;
        p.nickName = lp.username;
        gamePlayers.push_back(p);

        if (lp.playerId == state.playerId) {
            myIndex = i;
        }
    }

    SetupGame(gamePlayers, state.playerId);

    // Iniciar listener P2P
    NM.StartP2PListener(state.roomPlayers[myIndex].gamePort);

    for (int i = 0; i < (int)state.roomPlayers.size(); ++i)
    {
        if (i == myIndex) continue; // No conectar a ti mismo

        const auto& lp = state.roomPlayers[i];
        std::cout << "[CLIENT] Intentando conectar a " << lp.username
            << " (" << lp.ip << ":" << lp.gamePort << ")" << std::endl;

        for (int intento = 0; intento < 10; ++intento) {
            NM.AddConnection(lp.ip, lp.gamePort);
            sf::sleep(sf::milliseconds(100));
        }
    }

    std::cout << "[CLIENT] Conexiones establecidas: " << NM.GetConnections().size() << std::endl;
}

void GameScene::HandleEvent(const sf::Event& event) {
    if (event.is<sf::Event::MouseButtonPressed>())
    {
        const sf::Event::MouseButtonPressed* mbInfo =
            event.getIf<sf::Event::MouseButtonPressed>();

        if (mbInfo && mbInfo->button == sf::Mouse::Button::Left)
        {
            gameManager.TryPlacePieceScreen(static_cast<float>(mbInfo->position.x), static_cast<float>(mbInfo->position.y));
        }
    }
}

void GameScene::Update(float dt)
{
    NM.NetworkFetch();
    gameManager.ReceiveNetworkMoves();
    gameManager.Update(dt);
}

void GameScene::Render(sf::RenderWindow& window)
{
    gameManager.DrawGrid(window);
    gameManager.DrawHUD(window);
}

void GameScene::OnExit()
{
    std::cout << "Saliendo de GameScene..." << std::endl;
    NM.ClearConnections();
    gameManager.Reset();
}
