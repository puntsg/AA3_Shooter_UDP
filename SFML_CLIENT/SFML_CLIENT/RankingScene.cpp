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
        200.f, 500.f, 400.f, 50.f, font
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
    NM.SendRankingRequest(NM.GetClientState().nickname);
}

void RankingScene::HandleEvent(const sf::Event& event)
{
    if (backButton) backButton->handleEvent(event);
}

void RankingScene::Update(float dt)
{
    NM.NetworkFetch();
    auto& ranking = NM.GetClientState().ranking;
    if (!ranking.empty() && rankingData.empty())
    {
        for (int i = 0; i < (int)ranking.size(); i++)
            rankingData.push_back({ ranking[i].playerName, ranking[i].score });
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
    for (const auto& entry : rankingData)
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

    if (backButton) backButton->Draw(window);
}

void RankingScene::OnExit()
{
}
