#pragma once
#include <SFML/Graphics/Color.hpp>

namespace Config
{
    namespace Window
    {
        const int  WIDTH = 800;
        const int  HEIGHT = 600;
        inline const char* NAME = "Conecta3 Cliente";
        const int  FPS = 60;
    }

    namespace Connection
    {
        inline const char* SERVER_IP = "37.223.141.102";
        const unsigned short SERVER_PORT = 55000;
        const unsigned short GAME_PORT = 56000;

    }

    namespace Assets
    {
        inline const char* FONT_PATH = "assets/arial.ttf";
        inline const char* FONT_PATH_FALLBACK = "C:/Windows/Fonts/arial.ttf";
    }

    namespace Game
    {
        const short   GRID_COLUMNS = 6;
        const short   GRID_ROWS = 6;
        const short   CELL_SIZE = 90;
        const float MAX_TURN_TIME = 20.0f;
        const int   DEFAULT_SCORE = 1000;
        inline const char* DEFAULT_ROOM = "test_room";
        const int   WIN_COUNT = 3;
        const float HUD_TURN_X = 20.f;
        const float HUD_TURN_Y = 20.f;
        const float HUD_SCORE_X = 620.f;
        const float HUD_SCORE_SPACING_Y = 25.f;
        const int WIN_GAME = 20;
        const int LOSE_GAME = -5;


        //Colores jugadores
        inline const sf::Color PLAYER_COLORS[] = {
            sf::Color::Cyan,      // Jugador 1
            sf::Color::Red,       // Jugador 2
            sf::Color::Green,     // Jugador 3
            sf::Color::Yellow     // Jugador 4
        };
    }

    namespace UI
    {
        const short  FONT_SIZE_SMALL = 16;
        const short  FONT_SIZE_NORMAL = 18;
        const short  FONT_SIZE_MEDIUM = 20;
        const short  FONT_SIZE_TITLE = 40;
        const float TEXT_PADDING = 5.f;
        inline const sf::Color  COLOR_BACKGROUND{ 30,  30,  30 };
        inline const sf::Color  COLOR_CELL_EMPTY{ 50,  50,  50 };
        inline const sf::Color  COLOR_TEXT_DEFAULT = sf::Color::White;
        inline const sf::Color  COLOR_TEXT_ACCENT = sf::Color::Cyan;
    }

    namespace Login
    {
        const float INPUT_USERNAME_X = 250.f;
        const float INPUT_USERNAME_Y = 220.f;
        const float INPUT_PASSWORD_X = 250.f;
        const float INPUT_PASSWORD_Y = 250.f;
        const float INPUT_WIDTH = 300.f;
        const float INPUT_HEIGHT = 25.f;
        const float LOGIN_BUTTON_X = 350.f;
        const float LOGIN_BUTTON_Y = 280.f;
        const float REGISTER_BUTTON_X = 350.f;
        const float REGISTER_BUTTON_Y = 310.f;
        const float BUTTON_WIDTH = 100.f;
        const float BUTTON_HEIGHT = 25.f;
        const float CLOSE_BUTTON_SIZE = 64.f;
    }

    namespace Lobby
    {
        const float TITLE_X = 330.f;
        const float TITLE_Y = 100.f;
        const float SUBTITLE_X = 250.f;
        const float SUBTITLE_Y = 170.f;
		const float INPUT_X = 250.f;
		const float INPUT_Y = 200.f;
		const float INPUT_WIDTH = 300.f;
		const float INPUT_HEIGHT = 40.f;
		const float CREATE_BUTTON_X = 330.f;
		const float CREATE_BUTTON_Y = 250.f;
		const float RANKING_BUTTON_X = 330.f;
		const float RANKING_BUTTON_Y = 455.f;
		const float BUTTON_WIDTH = 140.f;
		const float BUTTON_HEIGHT = 45.f;
        const float STATUS_X = 230.f;
        const float STATUS_Y = 410.f;
        const float ROOM_INFO_X = 20.f;
        const float ROOM_INFO_Y = 20.f;
        const float BUTTON_PADDING = 10.f;
    }

    namespace Ranking
    {
        const float TITLE_X = 330.f;
        const float TITLE_Y = 100.f;
        const float TEXT_X = 200.f;
        const float TEXT_Y = 180.f;
        const float SPACING_Y = 30.f;
        const float BACK_BUTTON_X = 200.f;
        const float BACK_BUTTON_Y = 500.f;
        const float BACK_BUTTON_W = 400.f;
        const float BACK_BUTTON_H = 50.f;
    }
}

namespace GameConstants{
    namespace Player {
        const float GRAVITY_MULT = 10;
        const float JUMP_FORCE = 115;
        const float SPEED = 200;
    }
    namespace Health {
        const short INITIAL_LIFES = 3;
        const short INITIAL_HEALTH = 3;
    }
}
