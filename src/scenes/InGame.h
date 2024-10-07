#pragma once

#include "BaseScene.h"
#include "world/World.h"
#include "CameraUtils.h"
#include "ui/Panel.h"
#include "net/Client.h"


class InGame : public BaseScene
{
private:
    class OnClickLogout : public pk::ui::OnClickEvent
    {
    private:
        InGame& _sceneRef;

    public:
        OnClickLogout(InGame& sceneRef) :_sceneRef(sceneRef) {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    // Used atm temporarely to test world broadcasting!
    class OnMessageLogin_TEST : public net::OnMessageEvent
    {
    private:
        InGame& _sceneRef;

    public:
        OnMessageLogin_TEST(InGame& sceneRef) : _sceneRef(sceneRef) { }
        virtual void onMessage(const GC_byte* data, size_t dataSize);
    };

    class OnMessagePostLogin_TEST : public net::OnMessageEvent
    {
    private:
        InGame& _sceneRef;

    public:
        OnMessagePostLogin_TEST(
            InGame& sceneRef
        ) :
            _sceneRef(sceneRef)
        {}

        virtual void onMessage(const GC_byte* data, size_t dataSize);
    };

    class OnMessageLogout : public net::OnMessageEvent
    {
    public:
        OnMessageLogout() {}
        virtual void onMessage(const GC_byte* data, size_t dataSize);
    };

    Panel _mainPanel;

    world::World* _pWorld = nullptr;
    CameraController* _pCamController = nullptr;

    const size_t _observeAreaRadius = 15;
    const size_t _observeAreaWidth = _observeAreaRadius * 2 + 1;

public:
    std::string testUserName = "asd";
    std::string testUserPassword = "asd";

    bool waitingLogin = false;
    bool loggedIn = false;
    bool loggingOut = false;

    InGame();
    ~InGame();

    void init();
    void update();
    void lateUpdate();
};
