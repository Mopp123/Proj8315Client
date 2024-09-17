#include "BaseScene.h"
#include "../net/Client.h"
#include "core/Timing.h"


using namespace pk;
using namespace ui;
using namespace net;


BaseScene::BaseScene()
{}

BaseScene::~BaseScene()
{
    Client::get_instance()->clearOnMessageEvents();
}

void BaseScene::initBase()
{
    activeCamera = create_camera({0, 0, 0});
    _pInfoTxt = create_text(
        "",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, -100,
        ConstraintType::PIXEL_CENTER_VERTICAL, 120
    ).second;

    const vec2 debugPanelScale(300, 100);
    _debugPanel.init(
        { ConstraintType::PIXEL_RIGHT, 0.0f },
        { ConstraintType::PIXEL_TOP, 0.0f },
        vec2(debugPanelScale.x, 24),
        true, debugPanelScale
    );

    _pFPSCounterText = _debugPanel.addText("FPS HERE").second;

    _debugPanel.setActive(false);
}

void BaseScene::updateDebugPanel()
{
    // NOTE: Not sure is delta in milliseconds or what... so not sure if this works..
    const float delta = Timing::get_delta_time();
    if (delta > 0.0f)
    {
        float fps = 1000.0f / Timing::get_delta_time();
        _pFPSCounterText->accessStr() = "FPS: " + std::to_string(fps);
    }
}

void BaseScene::setInfoText(const std::string& txt, pk::vec3 color)
{
    _pInfoTxt->accessStr() = txt;
    _pInfoTxt->setColor(color);
}
