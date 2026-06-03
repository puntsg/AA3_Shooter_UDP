#include "RankingScene.h"
#include "Constants.h"

RankingScene::RankingScene()
{
    if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        std::cerr << "Warning: Failed to open arial.ttf font in Ranking" << std::endl;
    }
}

void RankingScene::BuildUI()
{
    backButton = std::make_unique<Button>(
        Config::Ranking::BACK_BUTTON_X,
        Config::Ranking::BACK_BUTTON_Y,
        Config::Ranking::BACK_BUTTON_W,
        Config::Ranking::BACK_BUTTON_H,
        font
    );
    backButton->SetText("Volver al Lobby");
    backButton->onClick = [this]() {
        SM.SetNextScene("LobbyScene");
    };
}

void RankingScene::OnEnter()
{
    BuildUI();
    rankingData.clear();
    NM.GetClientState().ResetRankingState();
    NM.GetClientState().rankingLoading = true;
    NM.GetClientState().rankingMessage = "Cargando ranking...";
    NM.SendRankingRequest(NM.GetClientState().nickname);
}

void RankingScene::HandleEvent(const sf::Event& event)
{
    if (backButton) backButton->handleEvent(event);
}

void RankingScene::Update(float dt)
{
    NM.NetworkFetch();
    ClientState& state = NM.GetClientState();
    std::vector<RankingData>& ranking = state.ranking;
    if (state.rankingReceived)
    {
        rankingData.clear();
        rankingData.reserve(ranking.size());
        for (const RankingData& entry : ranking)
            rankingData.emplace_back(entry.playerName, entry.score);

        state.rankingReceived = false;
    }
}

void RankingScene::Render(sf::RenderWindow& window)
{
    sf::Text title(font);
    title.setCharacterSize(Config::UI::FONT_SIZE_TITLE);
    title.setPosition({Config::Ranking::TITLE_X, Config::Ranking::TITLE_Y});
    title.setString("RANKING");
    title.setFillColor(sf::Color::Cyan);
    window.draw(title);

    float yPos = Config::Ranking::TEXT_Y;
    int position = 1;
    for (const std::pair<std::string, int>& entry : rankingData)
    {
        sf::Text rankText(font);
        rankText.setCharacterSize(Config::UI::FONT_SIZE_NORMAL);
        rankText.setFillColor(sf::Color::White);
        rankText.setPosition({ Config::Ranking::TEXT_X, yPos });
        std::string rankStr = std::to_string(position) + ". " + entry.first + " - " + std::to_string(entry.second);
        rankText.setString(rankStr);
        window.draw(rankText);

        yPos += Config::Ranking::SPACING_Y;
        position++;
    }

    const ClientState& state = NM.GetClientState();
    if (rankingData.empty() && !state.rankingMessage.empty())
    {
        sf::Text messageText(font);
        messageText.setCharacterSize(Config::UI::FONT_SIZE_NORMAL);
        messageText.setPosition({ Config::Ranking::TEXT_X, Config::Ranking::TEXT_Y });
        messageText.setString(state.rankingMessage);
        messageText.setFillColor(state.rankingMessageIsError ? sf::Color::Red : sf::Color::White);
        window.draw(messageText);
    }

    if (backButton) backButton->Draw(window);
}

void RankingScene::OnExit()
{
}
