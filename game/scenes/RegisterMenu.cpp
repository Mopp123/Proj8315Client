#include "RegisterMenu.h"
#include "MainMenu.h"
#include "LoginMenu.h"
#include "../world/Object.h"
#include "../world/Faction.h"
#include "../../Proj8315Common/src/messages/Message.h"


using namespace pk;
using namespace ui;
using namespace net;


void RegisterMenu::OnClickCancel::onClick(InputMouseButtonName button)
{
    if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        Application::get()->switchScene((Scene*)new LoginMenu);
}


void RegisterMenu::OnClickRegister::onClick(InputMouseButtonName button)
{
    if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
    {
        if (inputUsernameRef.size() > USER_NAME_SIZE)
        {
            ((BaseScene&)sceneRef).setInfoText(
                "Too long username.\nUsername has to be less than" +
                std::to_string(USER_NAME_SIZE) + " characters",
                vec3(1.0f, 0, 0)
            );
            return;
        }
        else if (inputPasswordRef.size() > USER_PASSWD_SIZE || inputRePasswordRef.size() > USER_PASSWD_SIZE)
        {
            ((BaseScene&)sceneRef).setInfoText(
                "Too long password.\nPassword has to be less than " +
                std::to_string(USER_PASSWD_SIZE) + " characters",
                vec3(1.0f, 0, 0)
            );
            return;
        }
        sceneRef.username = inputUsernameRef;
        Client::get_instance()->send(
            MESSAGE_TYPE__UserRegister,
            {
                { (GC_byte*)inputUsernameRef.data(), inputUsernameRef.size(), USER_NAME_SIZE },
                { (GC_byte*)inputPasswordRef.data(), inputPasswordRef.size(), USER_PASSWD_SIZE },
                { (GC_byte*)inputRePasswordRef.data(), inputRePasswordRef.size(), USER_PASSWD_SIZE }
            }
        );
    }
}


void RegisterMenu::OnMessageRegister::onMessage(const GC_byte* data, size_t dataSize)
{
    Debug::log("___TEST___RECV REGISTER RESPONSE!");
    Client* client = Client::get_instance();
    const size_t expectedSize = 1 + MESSAGE_INFO_MESSAGE_LEN;
    if (dataSize >= expectedSize)
    {
        bool success = *((bool*)data);
        GC_byte errMsgData[MESSAGE_INFO_MESSAGE_LEN];
        memset(errMsgData, 0, MESSAGE_INFO_MESSAGE_LEN);
        memcpy(errMsgData, data + 1, dataSize - 1);
        std::string errMsg(errMsgData, MESSAGE_INFO_MESSAGE_LEN);
        // On successful register -> attempt login immediately
        if (success)
        {
            Debug::log("___TEST___registering successful. Attempting to login..");

            ((BaseScene&)sceneRef).setInfoText("Registering was successful. Logging in...", vec3(0, 1.0f, 0));
            client->send(
                (int32_t)MESSAGE_TYPE__UserLogin,
                {
                    {
                        (GC_byte*)inputUsernameRef.data(),
                        inputUsernameRef.size(),
                        USER_NAME_SIZE
                    },
                    {
                        (GC_byte*)inputPasswordRef.data(),
                        inputPasswordRef.size(),
                        USER_PASSWD_SIZE
                    }
                }
            );
            }
        else
        {
            Debug::log("___TEST___failed login: recv error: " + errMsg);
            ((BaseScene&)sceneRef).setInfoText(errMsg, vec3(1.0f, 0, 0));
        }
    }
}


// TODO: Put into some "common OnMessage events" since this is used in multiple places(Login scene, register scene)
void RegisterMenu::OnMessageLoginRequest::onMessage(const GC_byte* data, size_t dataSize)
{
    Client* client = Client::get_instance();
    bool loginSuccess = *((bool*)data);
    bool hasFaction = *((bool*)data + 1);
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
void RegisterMenu::OnMessagePostLogin::onMessage(const GC_byte* data, size_t dataSize)
{
    world::objects::ObjectInfoLib::create(data, dataSize);
    Debug::log("Obj info lib created. Switching to main menu...");
    ((BaseScene&)sceneRef).setInfoText("Server data acquired", vec3(1.0f, 1.0f, 1.0f));
    Application::get()->switchScene((Scene*)(new MainMenu));
}


RegisterMenu::RegisterMenu()
{}

RegisterMenu::~RegisterMenu()
{}

void RegisterMenu::init()
{
    ((BaseScene*)this)->initBase();

    const float textSize = 32;
    const float rowPadding = 5;

    const float buttonWidth = 95;
    const float buttonHeight = 24;

    const float inputFieldWidth = 200;

    const float panelOffsetX = -100;
    const float panelOffsetY = 32;

    create_text(
        "Create new user",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY + textSize
    );

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

    std::pair<uint32_t, TextRenderable*> inputFieldRepeatPassword = create_input_field(
        "Re-enter password",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize * 2,
        inputFieldWidth,
        nullptr
    );

    std::string& usernameStr = inputFieldUsername.second->accessStr();
    std::string& passwdStr = inputFieldPassword.second->accessStr();
    std::string& rePasswdStr = inputFieldRepeatPassword.second->accessStr();

    create_button(
        "Register",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX + inputFieldWidth * 0.5f,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize * 3.0f,
        buttonWidth,
        buttonHeight,
        new OnClickRegister(*this, usernameStr, passwdStr, rePasswdStr)
    );
    create_button(
        "Cancel",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX,
        ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize * 3.0f,
        buttonWidth,
        buttonHeight,
        new OnClickCancel
    );

    Client* client = Client::get_instance();
    client->addOnMessageEvent(
        MESSAGE_TYPE__UserRegister,
        new OnMessageRegister(*this, usernameStr, passwdStr)
    );
    client->addOnMessageEvent(MESSAGE_TYPE__UserLogin, new OnMessageLoginRequest(*this));
    client->addOnMessageEvent(MESSAGE_TYPE__ObjInfoLibRequest, new OnMessagePostLogin(*this));
}

void RegisterMenu::update()
{}

