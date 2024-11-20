#pragma once

#include "BaseScene.h"
#include "world/World.h"
#include "CameraUtils.h"
#include "net/Client.h"
#include "world/MousePicker.h"
#include "ui/Panel.h"
#include "ui/InGameUI.h"


class InGame : public BaseScene
{
private:
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

    world::World* _pWorld = nullptr;
    CameraController* _pCamController = nullptr;
    world::MousePicker _mousePicker;

    const size_t _observeAreaRadius = 15;
    const size_t _observeAreaWidth = _observeAreaRadius * 2 + 1;

    InGameUI _inGameUI;

public:
    std::string testUserName = "asd";
    std::string testUserPassword = "asd";

    bool waitingLogin = false;
    bool loggedIn = false;
    bool loggingOut = false;

    InGame();
    ~InGame();

    // *Can create world only after ObjectInfoLib has been received from server and
    // created locally
    void createWorld();

    void init();
    void update();
    void lateUpdate();

    inline world::World* getWorld() { return _pWorld; }
    inline CameraController* getCamController() { return _pCamController; }
};
