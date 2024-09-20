#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "BaseScene.h"
#include "ui/Panel.h"


class MainMenu : public BaseScene
{
private:
    class OnClickLogin : public pk::ui::OnClickEvent
    {
    public:
        OnClickLogin();
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnClickOpenRegisterMenu : public pk::ui::OnClickEvent
    {
    private:
        MainMenu& _menuRef;
    public:
        OnClickOpenRegisterMenu(MainMenu& menuRef);
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

    class OnClickCancelRegister : public pk::ui::OnClickEvent
    {
    private:
        MainMenu& _menuRef;
    public:
        OnClickCancelRegister(MainMenu& menuRef);
        virtual void onClick(pk::InputMouseButtonName button);
    };

    Panel _mainPanel;
    Panel _serverInfoPanel;
    Panel _registerPanel;

    const int _mainMenuState = 0;
    const int _registerMenuState = 1;
    int _menuState = 0;

public:
    MainMenu();
    ~MainMenu();

    void init();
    void update();

    void setMainMenuActive();
    void setRegisterMenuActive();
};
