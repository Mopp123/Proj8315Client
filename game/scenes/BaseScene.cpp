#include "BaseScene.h"
#include "../net/Client.h"


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
}

void BaseScene::setInfoText(const std::string& txt, pk::vec3 color)
{
    _pInfoTxt->accessStr() = txt;
    _pInfoTxt->setColor(color);
}
