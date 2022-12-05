#include "MainMenu.h"
#include "../net/Client.h"
#include "../net/NetCommon.h"
#include "LoginMenu.h"


using namespace pk;
using namespace ui;
using namespace net;


class OnClickLogout : public OnClickEvent
{
public:
    OnClickLogout()
    {}

    // TODO: Send the actual logout message
    virtual void onClick(InputMouseButtonName button)
    {
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        {
            Application::get()->switchScene((Scene*)(new LoginMenu));
        }
    }
};


MainMenu::MainMenu()
{}

MainMenu::~MainMenu()
{}

void MainMenu::init()
{
    activeCamera = create_camera({0, 0, 0});

    vec2 originPos_infoPanel{100, 50};

    vec2 originPos_mainFuncPanel{100, 120};
    vec2 buttonScale_mainFuncPanel{100, 24};
    
    // Info panel
    create_text(
        "Server message of the day:\nNone",
        ConstraintType::PIXEL_LEFT, originPos_infoPanel.x,
        ConstraintType::PIXEL_TOP, originPos_infoPanel.y
    );


    // Main func panel
    _mainFuncPanel = new Panel(
        { ConstraintType::PIXEL_LEFT, 100},
        { ConstraintType::PIXEL_TOP, 150},
        { 130, 24 }
    );
    _mainFuncPanel->addButton("Enter Planet", nullptr);
    _mainFuncPanel->addButton("Options", nullptr);
    _mainFuncPanel->addButton("Logout", new OnClickLogout);


    // Faction panel
    create_input_field(
        "Faction", 
        ConstraintType::PIXEL_CENTER_HORIZONTAL, -20,
        ConstraintType::PIXEL_TOP, 150,
        200,
        nullptr,
        false
    );

    // Logout button
    /*
    create_button(
        "Log out",
        ConstraintType::PIXEL_LEFT, originPos_mainFuncPanel.x,
        ConstraintType::PIXEL_TOP, originPos_mainFuncPanel.y,
        buttonScale_mainFuncPanel.x, buttonScale_mainFuncPanel.y,
        new OnClickLogout
    );*/
}

void MainMenu::update()
{
}
