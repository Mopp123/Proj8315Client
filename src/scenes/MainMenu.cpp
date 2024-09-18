#include "MainMenu.h"
#include "net/Client.h"

using namespace pk;

MainMenu::MainMenu()
{
}

MainMenu::~MainMenu()
{
}

void MainMenu::init()
{
    initBase();

    setInfoText("Connecting...", vec3(1.0f, 0.75f, 0.0f));

    // testing panel creation..
    _mainPanel.create(
        this,
        HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL, 0,
        VerticalConstraintType::PIXEL_CENTER_VERTICAL, 0,
        { 200, 200 },
        Panel::LayoutFillType::VERTICAL,
        false,
        { 1.0f, 1.0f, 1.0f }, // color
        false,
        { 1, 0, 0, 0 } // border color
    );

    _mainPanel.addButton(
        "Test Button",
        nullptr,
        false,
        { 1, 0, 0 },
        *_pDefaultFont
    );

    std::pair<entityID_t, pk::TextRenderable*> inputFieldTest = pk::ui::create_input_field(
        "Very long input field info text here", *_pDefaultFont,
        pk::HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL, 0,
        pk::VerticalConstraintType::PIXEL_TOP, 0,
        200,
        nullptr,
        false
    );
}

void MainMenu::update()
{
    net::Client* pClient = net::Client::get_instance();
    if (pClient->isConnected())
        setInfoText("Connected!", vec3(0.0f, 1.0f, 0.0f));
}
