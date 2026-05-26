#include "TitleScene.h"
#include "SceneManager.h"

TitleScene::TitleScene()
{
    if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        std::cerr << "Warning: Failed to open arial.ttf font in Lobby" << std::endl;
    }

    playButton = new Button(Config::Login::REGISTER_BUTTON_X, Config::Login::REGISTER_BUTTON_Y, Config::Login::BUTTON_WIDTH, Config::Login::BUTTON_HEIGHT, font);
    playButton->SetText("Play");

    playButton->onClick = [this]() {
        SM.SetNextScene("GameScene");
     };
}

void TitleScene::OnEnter()
{
    std::cout << "Entrando a la TitleScene..." << std::endl;


}

void TitleScene::HandleEvent(const sf::Event& event)
{
    playButton->handleEvent(event);
}

void TitleScene::Update(float dt)
{
}

void TitleScene::Render(sf::RenderWindow& window)
{
    playButton->Draw(window);
}

void TitleScene::OnExit()
{
    std::cout << "Saliendo del Lobby..." << std::endl;
    delete playButton;
}