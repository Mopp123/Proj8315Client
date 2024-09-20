#include "MainMenu.h"
#include "net/Client.h"
#include "../../Proj8315Common/src/messages/Message.h"

using namespace pk;
using namespace net;


MainMenu::OnClickLogin::OnClickLogin()
{}

void MainMenu::OnClickLogin::onClick(pk::InputMouseButtonName button)
{}


MainMenu::OnClickOpenRegisterMenu::OnClickOpenRegisterMenu(MainMenu& menuRef) :
    _menuRef(menuRef)
{}

void MainMenu::OnClickOpenRegisterMenu::onClick(pk::InputMouseButtonName button)
{
    _menuRef.setRegisterMenuActive();
}


MainMenu::OnClickRegister::OnClickRegister(
    std::string& usernameRef,
    std::string& passwordRef,
    std::string& repasswordRef
) :
    _usernameRef(usernameRef),
    _passwordRef(passwordRef),
    _repasswordRef(repasswordRef)
{}

void MainMenu::OnClickRegister::onClick(pk::InputMouseButtonName button)
{
    Debug::log(
        "___TEST___registering user: " + _usernameRef + " "
        "password: " + _passwordRef
    );
    /*
    Client::get_instance()->send(
        MESSAGE_TYPE__UserRegisterRequest,
        {
            { (GC_byte*)_usernameRef.data(), _usernameRef.size(), USER_NAME_SIZE },
            { (GC_byte*)_passwordRef.data(), _passwordRef.size(), USER_PASSWD_SIZE },
            { (GC_byte*)_repasswordRef.data(), _repasswordRef.size(), USER_PASSWD_SIZE }
        }
    );
    */
}


MainMenu::OnClickCancelRegister::OnClickCancelRegister(MainMenu& menuRef) :
    _menuRef(menuRef)
{}

void MainMenu::OnClickCancelRegister::onClick(pk::InputMouseButtonName button)
{
    _menuRef.setMainMenuActive();
}


MainMenu::MainMenu()
{
}

MainMenu::~MainMenu()
{
}

void MainMenu::init()
{
    initBase();

    setInfoText("Connecting...", vec3(1.0f, 0.75f, 0.0f));

    // Main panel
    _mainPanel.createDefault(
        this,
        _pDefaultFont,
        HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL, -304,
        VerticalConstraintType::PIXEL_CENTER_VERTICAL, 115.0f * 0.5f,
        { 304, 115 },
        Panel::LayoutFillType::VERTICAL
    );

    _mainPanel.addDefaultInputField(
        "Username",
        200,
        nullptr
    );

    _mainPanel.addDefaultInputField(
        "Password",
        200,
        nullptr
    );

    const float buttonWidth = 297;
    _mainPanel.addDefaultButton(
        "Login",
        new OnClickLogin,
        buttonWidth
    );
    _mainPanel.addDefaultButton(
        "Reqister new user",
        new OnClickOpenRegisterMenu(*this),
        buttonWidth
    );

    // Register new user panel
    _registerPanel.createDefault(
        this,
        _pDefaultFont,
        HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL, -304,
        VerticalConstraintType::PIXEL_CENTER_VERTICAL, 115.0f * 0.5f,
        { 377, 143 },
        Panel::LayoutFillType::VERTICAL
    );

    std::pair<entityID_t, TextRenderable*> usernameInputField = _registerPanel.addDefaultInputField(
        "Username",
        272,
        nullptr
    );

    std::pair<entityID_t, TextRenderable*> passwordInputField = _registerPanel.addDefaultInputField(
        "Password",
        272,
        nullptr
    );

    std::pair<entityID_t, TextRenderable*> repasswordInputField = _registerPanel.addDefaultInputField(
        "Repeat password",
        200,
        nullptr
    );

    std::string& registerUsernameStrRef = usernameInputField.second->accessStr();
    std::string& registerPasswordStrRef = passwordInputField.second->accessStr();
    std::string& registerRepasswordStrRef = repasswordInputField.second->accessStr();

    _registerPanel.addDefaultButton(
        "Register",
        new OnClickRegister(
            registerUsernameStrRef,
            registerPasswordStrRef,
            registerRepasswordStrRef
        ),
        buttonWidth
    );
    _registerPanel.addDefaultButton(
        "Cancel",
        new OnClickCancelRegister(*this),
        buttonWidth
    );

    _serverInfoPanel.createDefault(
        this,
        _pDefaultFont,
        HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL, 4,
        VerticalConstraintType::PIXEL_CENTER_VERTICAL, 100,
        { 300, 200 },
        Panel::LayoutFillType::VERTICAL
    );
    _serverInfoPanel.addDefaultText(
        "Welcome!\n"
        "Some server message here..\n"
    );


    /*
    std::pair<entityID_t, pk::TextRenderable*> inputFieldTest2 = pk::ui::create_input_field(
        "Testing", *_pDefaultFont,
        HorizontalConstraintType::PIXEL_LEFT, 0,
        VerticalConstraintType::PIXEL_CENTER_VERTICAL, 0,
        400,
        nullptr, // on submit event
        false // clear on submit
    );
    */

    setMainMenuActive();
}

void MainMenu::update()
{
    InputManager* pInputManager = Application::get()->accessInputManager();

    net::Client* pClient = net::Client::get_instance();
    if (pClient->isConnected())
        setInfoText("Connected!", vec3(0.0f, 1.0f, 0.0f));
}

void MainMenu::setMainMenuActive()
{
    _mainPanel.setActive(true);
    _serverInfoPanel.setActive(true);
    _registerPanel.setActive(false);
}

void MainMenu::setRegisterMenuActive()
{
    _mainPanel.setActive(false);
    _serverInfoPanel.setActive(false);
    _registerPanel.setActive(true);
}
