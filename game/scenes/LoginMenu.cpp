#include "LoginMenu.h"
#include "../net/Client.h"
#include "../net/NetCommon.h"
#include "MainMenu.h"


using namespace pk;
using namespace ui;
using namespace net;


class OnMessageLoginRequest : public net::OnMessageEvent
{
public:
    LoginMenu& sceneRef;

    OnMessageLoginRequest(LoginMenu& scene) : 
        sceneRef(scene)
    {}

    virtual void onMessage(const PK_byte* data, size_t dataSize)
    {
        const size_t expectedDataSize = 1;
        if (dataSize >= expectedDataSize)
        {
            Client* client = Client::get_instance();
            bool loginSuccess = *((bool*)data);
            if (loginSuccess)
            {
                Debug::log("___TEST___LOGIN SUCCESS!");
                Application::get()->switchScene((Scene*)(new MainMenu));
                Debug::log("Login was success. Switching scene to MainMenu...");
            }
        }
    }
};


class OnClickLogin : public OnClickEvent
{
    private:
        std::string& _usernameFieldRef;
        std::string& _passwdFieldRef;

    public:
        OnClickLogin(std::string& usernameRef, std::string& passwdRef):
            _usernameFieldRef(usernameRef),
            _passwdFieldRef(passwdRef)
        {}

        virtual void onClick(InputMouseButtonName button)
        {
            Debug::log("___TEST___ON CLICK!!!");
            if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
            {
                const size_t usernameLen = _usernameFieldRef.length();
                const size_t passwdLen = _passwdFieldRef.length();

                if (usernameLen <= 0 || usernameLen > USER_NAME_LEN)
                {
                    Debug::log("Invalid username length", Debug::MessageType::PK_ERROR);
                    return;
                }
                if (passwdLen <= 0 || passwdLen > USER_NAME_LEN)
                {
                    Debug::log("Invalid password length", Debug::MessageType::PK_ERROR);
                    return;
                }

                size_t bufSize = sizeof(int32_t) + USER_NAME_LEN + USER_PASSWD_LEN;
                PK_byte* pSendBuf = new PK_byte[bufSize];
                memset(pSendBuf, 0, bufSize);

                const int32_t messageType = MESSAGE_TYPE__Login;
                memcpy(pSendBuf, &messageType, sizeof(int32_t));
                memcpy(pSendBuf + sizeof(int32_t), _usernameFieldRef.c_str(), usernameLen);
                memcpy(pSendBuf + sizeof(int32_t) + USER_NAME_LEN, _passwdFieldRef.c_str(), passwdLen);

                Client* client = Client::get_instance();
                client->send(pSendBuf, bufSize);

                delete[] pSendBuf;
            }
        }
};


LoginMenu::LoginMenu()
{}

LoginMenu::~LoginMenu()
{}

void LoginMenu::init()
{
    activeCamera = create_camera({0, 0, 0});

    const float textSize = 32;
    const float rowPadding = 5;

    const float buttonSize = 24;

    const float panelOffsetX = 0;
    const float panelOffsetY = 32;

    std::pair<uint32_t, TextRenderable*> inputFieldUsername = create_input_field(
        "Username",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY,
        200,
        nullptr
    );

    std::pair<uint32_t, TextRenderable*> inputFieldPassword = create_input_field(
        "Password",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize,
        200,
        nullptr
    );

    std::string& usernameStr = inputFieldUsername.second->accessStr();
    std::string& passwdStr = inputFieldPassword.second->accessStr();

    create_button(
        "Login",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, -panelOffsetX,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize * 2.0f,
        100,
        buttonSize,
        new OnClickLogin(usernameStr, passwdStr)
    );
    /*
    _loginButton = new Button(
        "Login",
        {
            {ConstraintType::PIXEL_CENTER_HORIZONTAL, -panelOffsetX},
            {ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize * 2.0f}
        },
        100,
        buttonSize,
        new OnClickLogin(_inputFieldUsername, _inputFieldPassword)
    );*/

    Client* client = Client::get_instance();
    client->addOnMessageEvent(MESSAGE_TYPE__Login, (OnMessageEvent*)(new OnMessageLoginRequest(*this)));
}

void LoginMenu::update()
{
}
