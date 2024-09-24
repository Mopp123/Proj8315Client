#include "MainMenu.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "../../Proj8315Common/src/messages/GeneralMessages.h"
#include "world/Objects.h"


using namespace pk;
using namespace net;
using namespace gamecommon;


void MainMenu::OnClickLogin::onClick(pk::InputMouseButtonName button)
{
    if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
    {
        _sceneRef.username = _usernameRef;
        _sceneRef.password = _passwordRef;
        const size_t usernameLen = _usernameRef.length();
        const size_t passwdLen = _passwordRef.length();

        if (usernameLen <= 0 || usernameLen > USER_NAME_SIZE)
        {
            Debug::log("Invalid username length", Debug::MessageType::PK_ERROR);
            return;
        }
        if (passwdLen <= 0 || passwdLen > USER_NAME_SIZE)
        {
            Debug::log("Invalid password length", Debug::MessageType::PK_ERROR);
            return;
        }

        _sceneRef.setInfoText("Waiting for server...", vec3(1.0f, 1.0f, 0.0f));

        Client* client = Client::get_instance();
        client->send(
            (int32_t)MESSAGE_TYPE__LoginRequest,
            {
                {
                    (GC_byte*)_usernameRef.data(),
                    usernameLen,
                    USER_NAME_SIZE
                },
                {
                    (GC_byte*)_passwordRef.data(),
                    passwdLen,
                    USER_PASSWD_SIZE
                }
            }
        );
    }
}


void MainMenu::OnClickOpenRegisterMenu::onClick(pk::InputMouseButtonName button)
{
    _menuRef.setRegisterMenuActive();
}

void MainMenu::OnClickCancelRegister::onClick(pk::InputMouseButtonName button)
{
    _menuRef.setMainMenuActive();
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
    Client::get_instance()->send(
        MESSAGE_TYPE__UserRegisterRequest,
        {
            { (GC_byte*)_usernameRef.data(), _usernameRef.size(), USER_NAME_SIZE },
            { (GC_byte*)_passwordRef.data(), _passwordRef.size(), USER_PASSWD_SIZE },
            { (GC_byte*)_repasswordRef.data(), _repasswordRef.size(), USER_PASSWD_SIZE }
        }
    );
}


void MainMenu::OnMessageRegister::onMessage(const GC_byte* data, size_t dataSize)
{
    Debug::log("___TEST___RECV REGISTER RESPONSE!");
    Client* client = Client::get_instance();
    const size_t expectedSize = 1 + MESSAGE_INFO_MESSAGE_LEN;
    if (dataSize >= expectedSize)
    {
        bool success = *((bool*)data + sizeof(uint32_t));
        size_t errMsgSize = dataSize - 1 - sizeof(uint32_t);
        GC_byte errMsgData[errMsgSize];
        memset(errMsgData, 0, errMsgSize);
        memcpy(errMsgData, data + sizeof(uint32_t) + 1, errMsgSize);
        char leading = errMsgData[0];
        Debug::log("___TEST___leading char = " + std::to_string(leading));
        std::string errMsg(errMsgData, errMsgSize);
        // On successful register -> attempt login immediately
        if (success)
        {
            Debug::log("___TEST___registering successful!");
            _sceneRef.triggerPopup();
        }
        else
        {
            Debug::log("___TEST___failed login: recv error: " + errMsg);
            _sceneRef.setRegisterInfoMsg(errMsg, vec3(1, 0, 0));
        }
    }
}


void MainMenu::OnMessageLogin::onMessage(const GC_byte* data, size_t dataSize)
{
    LoginResponse loginResponse(data, dataSize);
    if (loginResponse.getSuccess())
    {
        Client* client = Client::get_instance();

        client->user.name = _sceneRef.username;
        Faction userFaction = loginResponse.getFaction();

        if (userFaction != NULL_FACTION)
        {
            //client->user.faction = userFaction.getName();
            //client->user.hasFaction = true;
            Debug::log("___TEST___USER LOGGED IN WITH FACTION");
        }
        else
        {
            Debug::log("___TEST___USER LOGGED IN WITHOUT EXISTING FACTION!");
        }

        Debug::log("Login was success. Fetching server obj info lib...");
        _sceneRef.setInfoText(
            "Login success! \nFetching additional server data...",
            vec3(0.0f, 1.0f, 0),
            -304,
            115.0f * 0.5f + 40,
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL
        );
        client->send((int32_t)MESSAGE_TYPE__ObjInfoLibRequest, {});
    }
    else
    {
        _sceneRef.setInfoText(
            loginResponse.getError(),
            vec3(1.0f, 0, 0),
            -304,
            115.0f * 0.5f + 40,
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL
        );
    }
}


void MainMenu::OnMessageObjInfo::onMessage(const GC_byte* data, size_t dataSize)
{
    world::objects::ObjectInfoLib::create(data, dataSize);
    Debug::log("Obj info lib created. Switching to user menu");
    ((BaseScene&)_sceneRef).setInfoText("Server data acquired", vec3(0.0f, 1.0f, 0.0f));
    Application::get()->switchScene((Scene*)(new MainMenu));
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

    std::pair<entityID_t, TextRenderable*> usernameInputField = _mainPanel.addDefaultInputField(
        "Username",
        200,
        nullptr
    );

    std::pair<entityID_t, TextRenderable*> passwordInputField =_mainPanel.addDefaultInputField(
        "Password",
        200,
        nullptr
    );

    const float buttonWidth = 297;
    _mainPanel.addDefaultButton(
        "Login",
        new OnClickLogin(
            *this,
            usernameInputField.second->accessStr(),
            passwordInputField.second->accessStr()
        ),
        buttonWidth
    );
    Client::get_instance()->addOnMessageEvent(
        MESSAGE_TYPE__LoginResponse,
        new OnMessageLogin(*this)
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
        { 377, 170 },
        Panel::LayoutFillType::VERTICAL
    );

    std::pair<entityID_t, TextRenderable*> registerInfoText = _registerPanel.addText("Register new user", Panel::get_base_ui_color(3).toVec3());
    _registerInfoEntity = registerInfoText.first;

    std::pair<entityID_t, TextRenderable*> regUsernameInputField = _registerPanel.addDefaultInputField(
        "Username",
        272,
        nullptr
    );

    std::pair<entityID_t, TextRenderable*> regPasswordInputField = _registerPanel.addDefaultInputField(
        "Password",
        272,
        nullptr
    );

    std::pair<entityID_t, TextRenderable*> repasswordInputField = _registerPanel.addDefaultInputField(
        "Repeat password",
        200,
        nullptr
    );

    std::string& registerUsernameStrRef = regUsernameInputField.second->accessStr();
    std::string& registerPasswordStrRef = regPasswordInputField.second->accessStr();
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

    Client::get_instance()->addOnMessageEvent(
        MESSAGE_TYPE__UserRegisterResponse,
        new OnMessageRegister(*this)
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


    _popupInfoPanel.createDefault(
        this,
        _pDefaultFont,
        HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL, -100,
        VerticalConstraintType::PIXEL_CENTER_VERTICAL, 115.0f * 0.5f,
        { 200, 65 },
        Panel::LayoutFillType::VERTICAL
    );
    _popupInfoPanel.addDefaultText("Registration success");
    _popupInfoPanel.setActive(false);
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

    if (_displayRegisterInfoPopup)
    {
        _registerInfoPopupTimer -= 1.0f * Timing::get_delta_time();
        if (_registerInfoPopupTimer <= 0)
        {
            _registerInfoPopupTimer = _maxRegisterInfoPopupTimer;
            _displayRegisterInfoPopup = false;
            setMainMenuActive();
        }
        else
        {
            setPopupInfoActive();
        }
    }
}

void MainMenu::setMainMenuActive()
{
    // reset info
    setInfoText("");
    _mainPanel.setActive(true);
    _serverInfoPanel.setActive(true);
    _registerPanel.setActive(false);
    _popupInfoPanel.setActive(false);
}

void MainMenu::setRegisterMenuActive()
{
    // reset info
    setInfoText("");
    _mainPanel.setActive(false);
    _serverInfoPanel.setActive(false);
    _registerPanel.setActive(true);
    _popupInfoPanel.setActive(false);
}

void MainMenu::setPopupInfoActive()
{
    // reset info
    setInfoText("");
    _mainPanel.setActive(false);
    _serverInfoPanel.setActive(false);
    _registerPanel.setActive(false);
    _popupInfoPanel.setActive(true);
}

void MainMenu::setRegisterInfoMsg(const std::string& str, const pk::vec3& color)
{
    TextRenderable* pRenderable = (TextRenderable*)getComponent(
        _registerInfoEntity,
        ComponentType::PK_RENDERABLE_TEXT
    );
    pRenderable->accessStr() = str;
    pRenderable->color = color;
}
