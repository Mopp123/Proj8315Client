#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "BaseScene.h"
#include "ui/Panel.h"
#include "net/Client.h"


class MainMenu : public BaseScene
{
private:
    class OnClickLogin : public pk::ui::OnClickEvent
    {
    private:
        MainMenu& _sceneRef;
        std::string& _usernameRef;
        std::string& _passwordRef;

    public:
        OnClickLogin(
            MainMenu& sceneRef,
            std::string& usernameRef,
            std::string& passwordRef
        ) :
            _sceneRef(sceneRef),
            _usernameRef(usernameRef),
            _passwordRef(passwordRef)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnClickOpenRegisterMenu : public pk::ui::OnClickEvent
    {
    private:
        MainMenu& _menuRef;
    public:
        OnClickOpenRegisterMenu(MainMenu& menuRef) :
            _menuRef(menuRef)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnClickCancelRegister : public pk::ui::OnClickEvent
    {
    private:
        MainMenu& _menuRef;
    public:
        OnClickCancelRegister(MainMenu& menuRef) :
            _menuRef(menuRef)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnClickRegister : public pk::ui::OnClickEvent
    {
    private:
        std::string& _usernameRef;
        std::string& _passwordRef;
        std::string& _repasswordRef;
    public:
        OnClickRegister(
            std::string& usernameRef,
            std::string& passwordRef,
            std::string& repasswordRef
        );
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnMessageRegister : public net::OnMessageEvent
    {
    private:
        MainMenu& _sceneRef;

    public:
        OnMessageRegister(
            MainMenu& sceneRef
        ) :
            _sceneRef(sceneRef)
        {}

        virtual void onMessage(const GC_byte* data, size_t dataSize);
    };

    class OnMessageLogin : public net::OnMessageEvent
    {
    private:
        MainMenu& _sceneRef;

    public:
        OnMessageLogin(
            MainMenu& sceneRef
        ) :
            _sceneRef(sceneRef)
        {}

        virtual void onMessage(const GC_byte* data, size_t dataSize);
    };

    class OnMessagePostLogin : public net::OnMessageEvent
    {
    private:
        MainMenu& _sceneRef;

    public:
        OnMessagePostLogin(
            MainMenu& sceneRef
        ) :
            _sceneRef(sceneRef)
        {}

        virtual void onMessage(const GC_byte* data, size_t dataSize);
    };

    Panel _mainPanel;
    Panel _serverInfoPanel;
    Panel _registerPanel;
    Panel _popupInfoPanel;
    entityID_t _registerInfoEntity = NULL_ENTITY_ID;

    const int _mainMenuState = 0;
    const int _registerMenuState = 1;
    int _menuState = 0;

    bool _displayRegisterInfoPopup = false;
    const int _maxRegisterInfoPopupTimer = 70;
    int _registerInfoPopupTimer = _maxRegisterInfoPopupTimer;

public:
    std::string username;
    std::string password;

    MainMenu();
    ~MainMenu();

    void init();
    void update();

    void setMainMenuActive();
    void setRegisterMenuActive();
    void setPopupInfoActive();

    void setRegisterInfoMsg(const std::string& str, const pk::vec3& color);

    inline void triggerPopup() { _displayRegisterInfoPopup = true; }
};
