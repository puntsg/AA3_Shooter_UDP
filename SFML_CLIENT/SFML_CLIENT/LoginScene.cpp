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

        if (!ValidateCredentials(user, pass) || !EnsureServerConnection())
        {
            return;
        }

        NM.SendRegisterRequest(user, pass);
        };
    loginButton->onClick = [this]() {
        std::string user = usernameInputfield->getText();
        std::string pass = passwordInputfield->getText();

        if (!ValidateCredentials(user, pass) || !EnsureServerConnection())
        {
            return;
        }

        pendingLoginUser = user;
        if (NM.SendLoginRequest(user, pass))
        {
            NM.GetClientState().savedPassword = pass;
        }
    };
}

void LoginScene::OnEnter()
{
    std::cout << "Entrando a la LoginScene..." << std::endl;

    std::string savedUser = NM.GetClientState().nickname;
    std::string savedPass = NM.GetClientState().savedPassword;

    if (!savedUser.empty() && !savedPass.empty())
    {
        // Rellenar visualmente los campos por si acaso
        usernameInputfield->setText(savedUser);
        passwordInputfield->setText(savedPass);
        
        std::cout << "[CLIENT] Auto-login detectado. Conectando..." << std::endl;
        if (!NM.IsConnected()) {
            NM.ConnectToServer();
        }
        NM.SendLoginRequest(savedUser, savedPass);
    }
}

bool LoginScene::ValidateCredentials(const std::string& user, const std::string& pass)
{
    if (user.empty() || pass.empty())
    {
        NM.GetClientState().authMessage = "Usuario y password son obligatorios.";
        NM.GetClientState().authMessageIsError = true;
        return false;
    }

    return true;
}

bool LoginScene::EnsureServerConnection()
{
    if (NM.IsConnected())
    {
        return true;
    }

    if (!NM.ConnectToServer())
    {
        NM.GetClientState().authMessage = "No se pudo conectar con el servidor.";
        NM.GetClientState().authMessageIsError = true;
        return false;
    }

    return true;
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
    NM.NetworkFetch();
    if (NM.GetClientState().IsLoggedIn())
    {
        if (!pendingLoginUser.empty())
        {
            NM.GetClientState().nickname = pendingLoginUser;
            pendingLoginUser.clear();
        }
        SM.SetNextScene("LobbyScene");
    }
}

void LoginScene::Render(sf::RenderWindow& window)
{
    closeButton->Draw(window);
    usernameInputfield->Draw(window);
    passwordInputfield->Draw(window);
    loginButton->Draw(window);
    signinButton->Draw(window);

    const ClientState& state = NM.GetClientState();
    if (!state.authMessage.empty())
    {
        sf::Text messageText(font);
        messageText.setCharacterSize(Config::UI::FONT_SIZE_NORMAL);
        messageText.setPosition({ Config::Login::INPUT_USERNAME_X, Config::Login::REGISTER_BUTTON_Y + 40.f });
        messageText.setString(state.authMessage);
        messageText.setFillColor(state.authMessageIsError ? sf::Color::Red : sf::Color::Green);
        window.draw(messageText);
    }
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
