#include "RegisterMenu.h"
#include "MainMenu.h"
#include "LoginMenu.h"
#include "../world/Objects.h"
#include "../../Proj8315Common/src/Faction.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "../../Proj8315Common/src/messages/GeneralMessages.h"


using namespace pk;
using namespace ui;
using namespace net;
using namespace gamecommon;


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
            MESSAGE_TYPE__UserRegisterRequest,
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
            Debug::log("___TEST___registering successful. Attempting to login..");

            ((BaseScene&)sceneRef).setInfoText("Registering was successful. Logging in...", vec3(0, 1.0f, 0));
            client->send(
                (int32_t)MESSAGE_TYPE__LoginRequest,
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
void RegisterMenu::OnMessageLoginResponse::onMessage(const GC_byte* data, size_t dataSize)
{
    LoginResponse loginResponse(data, dataSize);
    if (loginResponse.getSuccess())
    {
        Client* client = Client::get_instance();
        client->user.name = sceneRef.username;
        // NOTE: User should never have existing faction when logging in first time after registration!!!
        Faction userFaction = loginResponse.getFaction();
        if (userFaction != NULL_FACTION)
        {
            client->user.faction = userFaction.getName();
            client->user.hasFaction = true;
        }

        Debug::log("Login was success. Fetching server obj info lib...");
        ((BaseScene&)sceneRef).setInfoText("Login success. Fetching additional server data...", vec3(1.0f, 1.0f, 1.0f));
        client->send((int32_t)MESSAGE_TYPE__ObjInfoLibRequest, {});
    }
    else
    {
        ((BaseScene&)sceneRef).setInfoText(loginResponse.getError(), vec3(1.0f, 0, 0));
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
        MESSAGE_TYPE__UserRegisterResponse,
        new OnMessageRegister(*this, usernameStr, passwdStr)
    );
    client->addOnMessageEvent(MESSAGE_TYPE__LoginResponse, new OnMessageLoginResponse(*this));
    client->addOnMessageEvent(MESSAGE_TYPE__ObjInfoLibResponse, new OnMessagePostLogin(*this));
}

void RegisterMenu::update()
{}

