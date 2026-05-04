#include "LoginScene.h"

LoginScene::LoginScene()
{
    if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        std::cerr << "Warning: Failed to open arial.ttf font in Lobby" << std::endl;
    }

    //ui setup
    usernameInputfield = new InputField(Config::Login::INPUT_USERNAME_X, Config::Login::INPUT_USERNAME_Y, Config::Login::INPUT_WIDTH, Config::Login::INPUT_HEIGHT, font);
    passwordInputfield = new InputField(Config::Login::INPUT_PASSWORD_X, Config::Login::INPUT_PASSWORD_Y, Config::Login::INPUT_WIDTH, Config::Login::INPUT_HEIGHT, font);
    loginButton = new Button(Config::Login::LOGIN_BUTTON_X, Config::Login::LOGIN_BUTTON_Y, Config::Login::BUTTON_WIDTH, Config::Login::BUTTON_HEIGHT, font);
    loginButton->SetText("Log In");
    signinButton = new Button(Config::Login::REGISTER_BUTTON_X, Config::Login::REGISTER_BUTTON_Y, Config::Login::BUTTON_WIDTH, Config::Login::BUTTON_HEIGHT, font);
    signinButton->SetText("Register");
    closeButton = new Button(Config::Window::WIDTH - Config::Login::CLOSE_BUTTON_SIZE, 0, Config::Login::CLOSE_BUTTON_SIZE, Config::Login::CLOSE_BUTTON_SIZE, font);
    closeButton->SetText("X");

    //onclic buttons 
    closeButton->onClick = []() {
        SM.window.close();
    };
    signinButton->onClick = [this]() {
        std::string user = usernameInputfield->getText();
        std::string pass = passwordInputfield->getText();
        };
    loginButton->onClick = [this]() {
        std::string user = usernameInputfield->getText();
        std::string pass = passwordInputfield->getText();
        
        // Guardar credenciales si no estan vacias
        if (!user.empty() && !pass.empty()) {
        }
    };
}

void LoginScene::OnEnter()
{
    std::cout << "Entrando a la LoginScene..." << std::endl;


}

void LoginScene::HandleEvent(const sf::Event& event)
{
    closeButton->handleEvent(event);
    usernameInputfield->handleEvent(event);
    passwordInputfield->handleEvent(event);
    loginButton->handleEvent(event);
    signinButton->handleEvent(event);
}

void LoginScene::Update(float dt)
{
}

void LoginScene::Render(sf::RenderWindow& window)
{
    closeButton->Draw(window);
    usernameInputfield->Draw(window);
    passwordInputfield->Draw(window);
    loginButton->Draw(window);
    signinButton->Draw(window);
}

void LoginScene::OnExit()
{
    std::cout << "Saliendo del Lobby..." << std::endl;
    delete loginButton;
    delete signinButton;
    delete usernameInputfield;
    delete passwordInputfield;
    delete closeButton;
}