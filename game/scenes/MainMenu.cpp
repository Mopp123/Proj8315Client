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

    const float buttonSize = 24;

    create_text(
        "Server message of the day:\nNone",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, -300,
        ConstraintType::PIXEL_CENTER_VERTICAL, 100
    );

    create_button(
        "Log out",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, 0,
        ConstraintType::PIXEL_CENTER_VERTICAL, -100,
        100,
        buttonSize,
        new OnClickLogout
    );
}

void MainMenu::update()
{
}
