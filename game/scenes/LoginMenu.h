#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "BaseScene.h"
#include "../net/Client.h"
#include "../../Proj8315Common/src/Common.h"


class LoginMenu : public BaseScene
{
private:
    // On click events
    class OnClickRegister : public pk::ui::OnClickEvent
    {
    public:
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnClickLogin : public pk::ui::OnClickEvent
    {
    public:
        LoginMenu& sceneRef;
        std::string& usernameFieldRef;
        std::string& passwdFieldRef;

        OnClickLogin(LoginMenu& scene, std::string& usernameRef, std::string& passwdRef):
            sceneRef(scene),
            usernameFieldRef(usernameRef),
            passwdFieldRef(passwdRef)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    // On message events
    // TODO: Put into some "common OnMessage events" since this is used in multiple places(Login scene, register scene)
    class OnMessageLoginRequest : public net::OnMessageEvent
    {
    public:
        LoginMenu& sceneRef;

        OnMessageLoginRequest(LoginMenu& scene) :
            sceneRef(scene)
        {}

        virtual void onMessage(const GC_byte* data, size_t dataSize);
    };

    // TODO: Put into some "common OnMessage events" since this is used in multiple places(Login scene, register scene)
    class OnMessagePostLogin : public net::OnMessageEvent
    {
    public:
        LoginMenu& sceneRef;

        OnMessagePostLogin(LoginMenu& sceneRef) :
            sceneRef(sceneRef)
        {}

        virtual void onMessage(const GC_byte* data, size_t dataSize);
    };


public:
    std::string username = "";
    bool _waitingForServer = false;

    LoginMenu();
    ~LoginMenu();

    virtual void init();
    virtual void update();
};
