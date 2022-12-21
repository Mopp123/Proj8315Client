#include "DeploymentMenu.h"
#include "MainMenu.h"


using namespace pk;
using namespace web;
using namespace ui;


class OnClickBack : public OnClickEvent
{
public:
    OnClickBack()
    {}

    virtual void onClick(InputMouseButtonName button)
    {
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        {
            Application::get()->switchScene((Scene*)(new MainMenu));
        }
    }
};


// TODO: Go to in game "arrival state",  using this
class OnClickDeploy : public OnClickEvent
{
public:
    OnClickDeploy()
    {}

    virtual void onClick(InputMouseButtonName button)
    {
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        {
        }
    }
};


DeploymentMenu::DeploymentMenu()
{}

DeploymentMenu::~DeploymentMenu()
{}

void DeploymentMenu::init()
{
    activeCamera = create_camera({0, 0, 0}, 0.0f, 0.0f);

    vec2 buttonScale(100, 24);
    
    create_image(
        ConstraintType::PIXEL_CENTER_HORIZONTAL, 0,
        ConstraintType::PIXEL_CENTER_VERTICAL, 0,
        400, 300, 
        false, 
        { 0.1f, 0.1f, 0.1f }
    );

    create_button(
        "Back",
        ConstraintType::PIXEL_LEFT, 120,
        ConstraintType::PIXEL_BOTTOM, 120,
        buttonScale.x, buttonScale.y,
        new OnClickBack
    );

    create_button(
        "Deploy",
        ConstraintType::PIXEL_RIGHT, 120,
        ConstraintType::PIXEL_BOTTOM, 120,
        buttonScale.x, buttonScale.y,
        nullptr
    );
}

void DeploymentMenu::update()
{}
