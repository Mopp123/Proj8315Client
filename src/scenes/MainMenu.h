#pragma once

#include "pesukarhu/ppk.h"
#include "BaseScene.h"
#include "net/Client.h"
#include "pesukarhu/ui/InputField.h"


class MainMenu : public BaseScene
{
private:
    class OnClickLogin : public pk::ui::GUIButton::OnClickEvent
    {
    private:
        MainMenu& _sceneRef;
        pk::ui::InputField _usernameInputField;
        pk::ui::InputField _passwordInputField;

    public:
        OnClickLogin(
            MainMenu& sceneRef,
            pk::ui::InputField usernameInputField,
            pk::ui::InputField passwordInputField
        ) :
            _sceneRef(sceneRef),
            _usernameInputField(usernameInputField),
            _passwordInputField(passwordInputField)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnClickOpenRegisterMenu : public pk::ui::GUIButton::OnClickEvent
    {
    private:
        MainMenu& _menuRef;
    public:
        OnClickOpenRegisterMenu(MainMenu& menuRef) :
            _menuRef(menuRef)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnClickCancelRegister : public pk::ui::GUIButton::OnClickEvent
    {
    private:
        MainMenu& _menuRef;
    public:
        OnClickCancelRegister(MainMenu& menuRef) :
            _menuRef(menuRef)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnClickRegister : public pk::ui::GUIButton::OnClickEvent
    {
    private:
        MainMenu& _sceneRef;
        pk::ui::InputField _usernameInputField;
        pk::ui::InputField _passwordInputField;
        pk::ui::InputField _repasswordInputField;
    public:
        OnClickRegister(
            MainMenu& sceneRef,
            pk::ui::InputField usernameInputField,
            pk::ui::InputField passwordInputField,
            pk::ui::InputField repasswordInputField
        ) :
            _sceneRef(sceneRef),
            _usernameInputField(usernameInputField),
            _passwordInputField(passwordInputField),
            _repasswordInputField(repasswordInputField)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };


    class OnMessageServerInfo : public net::OnMessageEvent
    {
    public:
        MainMenu& sceneRef;
        OnMessageServerInfo(MainMenu& sceneRef) : sceneRef(sceneRef) {}
        virtual void onMessage(const GC_byte* data, size_t dataSize);
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

    pk::ui::Panel _mainPanel;
    pk::ui::Panel _serverInfoPanel;
    pk::ui::Panel _registerPanel;
    pk::ui::Panel _popupInfoPanel;

    entityID_t _registerInfoEntity = NULL_ENTITY_ID;
    entityID_t _serverInfoTxtEntity = NULL_ENTITY_ID;

    const int _mainMenuState = 0;
    const int _registerMenuState = 1;
    int _menuState = 0;

    bool _displayRegisterInfoPopup = false;
    const int _maxRegisterInfoPopupTimer = 70;
    int _registerInfoPopupTimer = _maxRegisterInfoPopupTimer;

    bool _isServerInfoRequested = false;

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

    void setServerInfoMessage(const std::string& message);

    inline void triggerPopup() { _displayRegisterInfoPopup = true; }
};
