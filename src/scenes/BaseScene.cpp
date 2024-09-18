#include "BaseScene.h"
#include "core/Application.h"
#include "core/Debug.h"
#include "net/Client.h"


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
    activeCamera = createEntity();
    createCamera(activeCamera, { 0, 1.0f, 4.0f }, 0.0f, 0.0f);
    pk::Debug::log("@BaseScene::initBase Loading default font");
    pk::Application* pApp = pk::Application::get();
    pk::ResourceManager& resourceManager = pApp->getResourceManager();
    _pDefaultFont = resourceManager.createFont(
        "assets/Ubuntu-R.ttf",
        20
    );

    // Create directional light
    directionalLight = createEntity();
    pk::vec3 lightDirection(0.33f, -0.33f, -0.33f);
    lightDirection.normalize();
    createDirectionalLight(
        directionalLight,
        { 1.0f, 1.0f, 1.0f },
        lightDirection
    );

    _pInfoTxt = create_text(
        "", *_pDefaultFont,
        HorizontalConstraintType::PIXEL_LEFT, 0,
        VerticalConstraintType::PIXEL_TOP, 0
    ).second;
}

void BaseScene::setInfoText(const std::string& txt, pk::vec3 color)
{
    _pInfoTxt->accessStr() = txt;
    _pInfoTxt->setColor(color);
}
