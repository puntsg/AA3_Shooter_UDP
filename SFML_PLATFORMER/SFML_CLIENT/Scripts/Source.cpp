#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "../Scripts/Scenes/SceneManager.h"
#include "../Scripts/Scenes/GameScene.h"
#include "../Scripts/Scenes/TitleScene.h"

int main()
{
    TitleScene* titleScene = new TitleScene();
    GameScene* gameScene = new GameScene();
    SM.window = sf::RenderWindow(sf::VideoMode({ Config::Window::WIDTH,Config::Window::HEIGHT }), Config::Window::NAME);
    SM.window.setFramerateLimit(Config::Window::FPS);
    SM.AddScene("TitleScene", titleScene);
    SM.AddScene("GameScene", gameScene);
    SM.InitFirstScene("TitleScene");

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