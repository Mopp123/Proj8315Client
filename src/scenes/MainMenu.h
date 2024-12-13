#pragma once

#include "pesukarhu/ppk.h"
#include "BaseScene.h"
#include "net/Client.h"
#include "pesukarhu/ui/GUIText.h"
#include "pesukarhu/ui/InputField.h"


class MainMenu : public BaseScene
{
private:
    class OnClickLogin : public pk::ui::GUIButton::OnClickEvent
    {
    private:
        MainMenu& _sceneRef;
        pk::ui::InputField* _pUsernameInputField;
        pk::ui::InputField* _pPasswordInputField;

    public:
        OnClickLogin(
            MainMenu& sceneRef,
            pk::ui::InputField* pUsernameInputField,
            pk::ui::InputField* pPasswordInputField
        ) :
            _sceneRef(sceneRef),
            _pUsernameInputField(pUsernameInputField),
            _pPasswordInputField(pPasswordInputField)
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
        pk::ui::InputField* _pUsernameInputField;
        pk::ui::InputField* _pPasswordInputField;
        pk::ui::InputField* _pRepasswordInputField;
    public:
        OnClickRegister(
            MainMenu& sceneRef,
            pk::ui::InputField* pUsernameInputField,
            pk::ui::InputField* pPasswordInputField,
            pk::ui::InputField* pRepasswordInputField
        ) :
            _sceneRef(sceneRef),
            _pUsernameInputField(pUsernameInputField),
            _pPasswordInputField(pPasswordInputField),
            _pRepasswordInputField(pRepasswordInputField)
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

    pk::ui::GUIText* _pRegisterInfoEntity = nullptr;
    pk::ui::GUIText* _pServerInfoTxtEntity = nullptr;

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
