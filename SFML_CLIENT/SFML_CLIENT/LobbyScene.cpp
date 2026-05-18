#include "LobbyScene.h"
#include "LoginScene.h"

LobbyScene::LobbyScene()
{
    if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        std::cerr << "Warning: Failed to open arial.ttf font in Lobby" << std::endl;
    }
}

void LobbyScene::BuildUI()
{
	roomIdInput = std::make_unique<InputField>(
        Config::Lobby::INPUT_X, 
        Config::Lobby::INPUT_Y, 
        Config::Lobby::INPUT_WIDTH, 
        Config::Lobby::INPUT_HEIGHT, 
        font);

	createButton = std::make_unique<Button>(
        Config::Lobby::CREATE_BUTTON_X, 
        Config::Lobby::CREATE_BUTTON_Y, 
        Config::Lobby::BUTTON_WIDTH, 
        Config::Lobby::BUTTON_HEIGHT, 
		font);
	createButton->SetText("Buscar amistosa");
	createButton->onClick = [this]() 
        { 
            AskNormalMatchmaking(); 
        };

    joinButton = std::make_unique<Button>(
        Config::Lobby::CREATE_BUTTON_X, 
        Config::Lobby::CREATE_BUTTON_Y + Config::Lobby::BUTTON_HEIGHT + 10.f, 
        Config::Lobby::BUTTON_WIDTH, 
		Config::Lobby::BUTTON_HEIGHT,
		font);
	joinButton->SetText("Buscar ranked");
    joinButton->onClick = [this]() 
        { 
            AskRankedMatchmaking(); 
		};

    rankingButton = std::make_unique<Button>(
        Config::Lobby::RANKING_BUTTON_X, 
        Config::Lobby::RANKING_BUTTON_Y, 
        Config::Lobby::BUTTON_WIDTH, 
        Config::Lobby::BUTTON_HEIGHT,
        font);
    rankingButton->SetText("Ranking");
    rankingButton->onClick = [this]() {
        SM.SetNextScene("RankingScene");
    };

	uiInitialized = true;
}

void LobbyScene::OnEnter()
{
    std::cout << "Entrando al LobbyScene..." << std::endl;

    if(!uiInitialized)
		BuildUI();

	auto& state = NM.GetClientState();

    //limpiar estado visual lobby
	state.hasGameStarted = false;
	state.isWaitingInRoom = false;
	state.currentRoomId.clear();
	state.roomPlayers.clear();
	state.isHost = false;

	unsigned short myPort = static_cast<unsigned short>(55001 + state.playerId);
	state.myGamePort = myPort;

	std::cout << "[Client] P2P port asigned: " << myPort << std::endl;
	statusText = "Selecciona una cola para buscar partida";
}

void LobbyScene::HandleEvent(const sf::Event& event)
{
    if (roomIdInput) roomIdInput->handleEvent(event);
    if (createButton) createButton->handleEvent(event);
    if (joinButton) joinButton->handleEvent(event);
    if (rankingButton) rankingButton->handleEvent(event);

    if (event.is<sf::Event::KeyPressed>())
    {
        const sf::Event::KeyPressed* kpInfo = event.getIf<sf::Event::KeyPressed>();
		if (!kpInfo) return;

        if (kpInfo->code == sf::Keyboard::Key::Enter)
        {
			AskNormalMatchmaking();
        }

        if (kpInfo->code == sf::Keyboard::Key::Escape)
        {
			statusText = "Saliendo del lobby...";
        }
    }
}

void LobbyScene::AskNormalMatchmaking()
{
    auto& state = NM.GetClientState();
    NM.SendMatchmakingRequest(false, state.nickname, state.myGamePort);
    statusText = "Buscando partida amistosa...";
    std::cout << "[CLIENT] Matchmaking amistoso solicitado." << std::endl;
}

void LobbyScene::AskRankedMatchmaking()
{
    auto& state = NM.GetClientState();
    NM.SendMatchmakingRequest(true, state.nickname, state.myGamePort);
    statusText = "Buscando partida ranked...";
    std::cout << "[CLIENT] Matchmaking ranked solicitado." << std::endl;
}

void LobbyScene::AskCreateRoom()
{
    if (!roomIdInput) return;

    std::string roomId = roomIdInput->getText();
    if (roomId.empty())
    {
        statusText = "El ID de la sala no puede estar vacio";
        std::cout << "[CLIENT] roomId vacio al crear sala" << std::endl;
        return;
    }

    auto& state = NM.GetClientState();
    NM.SendCreateRoomRequest(roomId, state.nickname, state.myGamePort);

    statusText = "Solicitando crear sala: " + roomId;
    std::cout << "[CLIENT] CREATE_ROOM_REQUEST enviado para sala " << roomId << std::endl;
}

void LobbyScene::AskJoinRoom()
{
    if (!roomIdInput) return;

    std::string roomId = roomIdInput->getText();
    if (roomId.empty())
    {
        statusText = "El ID de la sala no puede estar vacio";
        std::cout << "[CLIENT] roomId vacio al unirse." << std::endl;
        return;
    }

    auto& state = NM.GetClientState();
    NM.SendJoinRoomRequest(roomId, state.nickname, state.myGamePort);

    statusText = "Solicitando unirse a sala: " + roomId;
    std::cout << "[CLIENT] JOIN_ROOM_REQUEST enviado para sala " << roomId << std::endl;
}

void LobbyScene::Update(float dt)
{
    NM.NetworkFetch();

    const auto& state = NM.GetClientState();

    if (state.hasGameStarted)
    {
        statusText = "Partida iniciada...";
        SM.SetNextScene("GameScene");
        return;
    }

    if (state.isWaitingInRoom)
    {
        statusText = "Esperando jugadores en sala: " + state.currentRoomId;
    }
}

void LobbyScene::Render(sf::RenderWindow& window)
{
    sf::Text titleText(font);
    titleText.setCharacterSize(Config::UI::FONT_SIZE_TITLE);
    titleText.setPosition({ Config::Lobby::TITLE_X, Config::Lobby::TITLE_Y });
    titleText.setString("LOBBY");
    titleText.setFillColor(sf::Color::White);

    sf::Text labelText(font);
    labelText.setCharacterSize(Config::UI::FONT_SIZE_MEDIUM);
    labelText.setPosition({ Config::Lobby::SUBTITLE_X, Config::Lobby::SUBTITLE_Y });
    labelText.setString("Matchmaking:");
    labelText.setFillColor(sf::Color::White);

    sf::Text status(font);
    status.setCharacterSize(Config::UI::FONT_SIZE_NORMAL);
    status.setPosition({ Config::Lobby::STATUS_X, Config::Lobby::STATUS_Y });
    status.setString(statusText);
    status.setFillColor(sf::Color::Cyan);
    
	window.draw(titleText);
    window.draw(labelText);

    if (createButton) createButton->Draw(window);
	if (joinButton) joinButton->Draw(window);
    if (rankingButton) rankingButton->Draw(window);

	window.draw(status);

	const auto& state = NM.GetClientState();

	sf::Text roomInfo(font);
	roomInfo.setCharacterSize(Config::UI::FONT_SIZE_NORMAL);
	roomInfo.setFillColor(sf::Color::White);
	roomInfo.setPosition({ Config::Lobby::ROOM_INFO_X, Config::Lobby::ROOM_INFO_Y });

	std::string info = "Nick: " + state.nickname + "\n";

    if(state.isWaitingInRoom)
    {
        info += "Sala: " + state.currentRoomId + "\n";
		info += state.isHost ? "Host\n" : "Cliente\n";
        info += "Jugadores en sala:" + std::to_string(state.roomPlayers.size()) + "\n";

        for (const auto& player : state.roomPlayers)
        {
            info += " - " + player.username;
			if (player.isHost) info += " (Host)";
			info += "\n";
        }
	}

	roomInfo.setString(info);
	window.draw(roomInfo);
}

void LobbyScene::OnExit()
{
    std::cout << "Saliendo del Lobby..." << std::endl;
}

