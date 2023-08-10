#include "LoginMenu.h"
#include "../net/Client.h"
#include "../net/NetCommon.h"
#include "../world/Object.h"
#include "../world/Faction.h"
#include "MainMenu.h"
#include "RegisterMenu.h"

#include "../../Proj8315Common/src/messages/Message.h"


using namespace pk;
using namespace ui;
using namespace net;


void LoginMenu::OnClickRegister::onClick(InputMouseButtonName button)
{
    if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        Application::get()->switchScene((Scene*)(new RegisterMenu));
}


void LoginMenu::OnClickLogin::onClick(InputMouseButtonName button)
{
    if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
    {
        const size_t usernameLen = usernameFieldRef.length();
        const size_t passwdLen = passwdFieldRef.length();

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
        sceneRef._waitingForServer = true;
        sceneRef.username = usernameFieldRef;
        Client* client = Client::get_instance();
        client->send(
            (int32_t)MESSAGE_TYPE__UserLogin,
            {
                {
                    (GC_byte*)usernameFieldRef.data(),
                    usernameLen,
                    USER_NAME_SIZE
                },
                {
                    (GC_byte*)passwdFieldRef.data(),
                    passwdLen,
                    USER_PASSWD_SIZE
                }
            }
        );
    }
}


// TODO: Put into some "common OnMessage events" since this is used in multiple places(Login scene, register scene)
void LoginMenu::OnMessageLoginRequest::onMessage(const GC_byte* data, size_t dataSize)
{
    Client* client = Client::get_instance();
    // NEW

    // OLD BELOW
    bool loginSuccess = *((bool*)data);
    bool hasFaction = *((bool*)data + 1);
    const size_t expectedSize = 2 + FACTION_NAME_SIZE;
    if (loginSuccess)
    {
        if (dataSize > 2)
        {
            // TODO: Make it so that on login response server sends username/key of the
            // successfully logged in user gets put as client's username/key
            client->user.name = sceneRef.username;

            Faction faction(data + 2);
            client->user.faction = faction.getName();
            client->user.hasFaction = hasFaction;
        }
        Debug::log("Login was success. Fetching server obj info lib...");
        ((BaseScene&)sceneRef).setInfoText("Login success. Fetching additional server data...", vec3(1.0f, 1.0f, 1.0f));
        client->send((int32_t)MESSAGE_TYPE__ObjInfoLibRequest, {});
    }
    else
    {
        const size_t errMessagePos = 2 + Faction::get_netw_size();
        const size_t errMessageSize = dataSize - errMessagePos;
        if (dataSize > errMessagePos)
        {
            char infoMessage[MESSAGE_INFO_MESSAGE_LEN];
            memset(infoMessage, 0, MESSAGE_INFO_MESSAGE_LEN);
            memcpy(infoMessage, data + errMessagePos, errMessageSize);
            ((BaseScene&)sceneRef).setInfoText(std::string(infoMessage), vec3(1.0f, 0, 0));
        }
        else
        {
            Debug::log("Failed to login but server didn't provide error/info message", Debug::MessageType::PK_WARNING);
        }
    }
}


// After successful "login validation" -> we need to get Object Info lib
// TODO: Put into some "common OnMessage events" since this is used in multiple places(Login scene, register scene)
void LoginMenu::OnMessagePostLogin::onMessage(const GC_byte* data, size_t dataSize)
{
    world::objects::ObjectInfoLib::create(data, dataSize);
    Debug::log("Obj info lib created. Switching to main menu...");
    ((BaseScene&)sceneRef).setInfoText("Server data acquired", vec3(1.0f, 1.0f, 1.0f));
    Application::get()->switchScene((Scene*)(new MainMenu));
}


LoginMenu::LoginMenu()
{}

LoginMenu::~LoginMenu()
{}

void LoginMenu::init()
{
    ((BaseScene*)this)->initBase();

    const float textSize = 32;
    const float rowPadding = 5;

    const float buttonWidth = 95;
    const float buttonHeight = 24;

    const float inputFieldWidth = 200;

    const float panelOffsetX = -100;
    const float panelOffsetY = 32;

    std::pair<uint32_t, TextRenderable*> inputFieldUsername = create_input_field(
        "Username",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY,
        inputFieldWidth,
        nullptr
    );

    std::pair<uint32_t, TextRenderable*> inputFieldPassword = create_input_field(
        "Password",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize,
        inputFieldWidth,
        nullptr
    );

    std::string& usernameStr = inputFieldUsername.second->accessStr();
    std::string& passwdStr = inputFieldPassword.second->accessStr();

    create_button(
        "Login",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX + inputFieldWidth * 0.5f + 12,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize * 2.0f,
        buttonWidth,
        buttonHeight,
        new OnClickLogin(*this, usernameStr, passwdStr)
    );

    create_text(
        "New user?",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize * 3.0f
    );

    create_button(
        "Register",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX + inputFieldWidth * 0.5f + 12,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize * 3.0f,
        buttonWidth,
        buttonHeight,
        new OnClickRegister
    );

    _pInfoTxt->setActive(false);

    Client* client = Client::get_instance();
    client->addOnMessageEvent(MESSAGE_TYPE__UserLogin, (OnMessageEvent*)(new OnMessageLoginRequest(*this)));
    client->addOnMessageEvent(MESSAGE_TYPE__ObjInfoLibRequest, (OnMessageEvent*)(new OnMessagePostLogin(*this)));
}

void LoginMenu::update()
{
    if (_waitingForServer)
        _pInfoTxt->setActive(true);
}

