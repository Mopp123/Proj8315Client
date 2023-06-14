#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "BaseScene.h"
#include "../net/NetCommon.h"
#include "../net/Client.h"
#include "../ui/ui.h"


class RegisterMenu : public BaseScene
{
private:
    // On click events
    class OnClickRegister : public pk::ui::OnClickEvent
    {
    public:
        RegisterMenu& sceneRef;
        std::string& inputUsernameRef;
        std::string& inputPasswordRef;
        std::string& inputRePasswordRef;

    OnClickRegister(
        RegisterMenu& sceneRef,
        std::string& inputUsernameRef,
        std::string& inputPasswordRef,
        std::string& inputRePasswordRef
    ) :
        sceneRef(sceneRef),
        inputUsernameRef(inputUsernameRef),
        inputPasswordRef(inputPasswordRef),
        inputRePasswordRef(inputRePasswordRef)
    {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnClickCancel : public pk::ui::OnClickEvent
    {
    public:
        virtual void onClick(pk::InputMouseButtonName button);
    };

    // On message events
    class OnMessageRegister : public net::OnMessageEvent
    {
    public:
        RegisterMenu& sceneRef;
        std::string& inputUsernameRef;
        std::string& inputPasswordRef;

        OnMessageRegister(
            RegisterMenu& sceneRef,
            std::string& inputUsernameRef,
            std::string& inputPasswordRef
        ) :
            sceneRef(sceneRef),
            inputUsernameRef(inputUsernameRef),
            inputPasswordRef(inputPasswordRef)
        {}

        virtual void onMessage(const PK_byte* data, size_t dataSize);
    };

    // TODO: Put into some "common OnMessage events" since this is used in multiple places(Login scene, register scene)
    class OnMessageLoginRequest : public net::OnMessageEvent
    {
    public:
        RegisterMenu& sceneRef;

        OnMessageLoginRequest(RegisterMenu& scene) :
            sceneRef(scene)
        {}

        virtual void onMessage(const PK_byte* data, size_t dataSize);
    };

    // TODO: Put into some "common OnMessage events" since this is used in multiple places(Login scene, register scene)
    class OnMessagePostLogin : public net::OnMessageEvent
    {
    public:
        RegisterMenu& sceneRef;

        OnMessagePostLogin(RegisterMenu& sceneRef) :
            sceneRef(sceneRef)
        {}

        virtual void onMessage(const PK_byte* data, size_t dataSize);
    };

public:
    std::string username = "";

    RegisterMenu();
    ~RegisterMenu();

    virtual void init();
    virtual void update();
};

