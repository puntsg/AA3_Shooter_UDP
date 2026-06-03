#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "NetworkManager.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "LobbyScene.h"
#include "LoginScene.h"
#include "RankingScene.h"
#include "LauncherManager.h"

int main()
{
    LauncherManager::RunLauncher();
    LoginScene* loginScene = new LoginScene();
    GameScene* gameScene = new GameScene();
    RankingScene* rankingScene = new RankingScene();
    LobbyScene* lobbyScene = new LobbyScene();
    SM.window = sf::RenderWindow(sf::VideoMode({ Config::Window::WIDTH,Config::Window::HEIGHT }), Config::Window::NAME);
    SM.window.setFramerateLimit(Config::Window::FPS);
    SM.AddScene("LoginScene", loginScene);
    SM.AddScene("GameScene", gameScene);
    SM.AddScene("LobbyScene", lobbyScene);
    SM.AddScene("RankingScene", rankingScene);
    SM.InitFirstScene("LoginScene");

    if (!NM.ConnectToServer())
    {
        //return -1;
    }

    sf::Clock dtClock;
    while (SM.window.isOpen())
    {
        float dt = dtClock.restart().asSeconds();

        while (const std::optional<sf::Event> event = SM.window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                SM.window.close();


            if (SM.GetCurrentScene())
                SM.GetCurrentScene()->HandleEvent(*event);
        }

        SM.UpdateCurrentScene(dt);
        SM.window.clear(Config::UI::COLOR_BACKGROUND);

        if (SM.GetCurrentScene())
            SM.GetCurrentScene()->Render(SM.window);

        SM.window.display();
    }

    return 0;
}
