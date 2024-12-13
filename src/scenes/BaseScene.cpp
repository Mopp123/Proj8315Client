#include "BaseScene.h"
#include "net/Client.h"

using namespace pk;
using namespace ui;
using namespace net;


BaseScene::BaseScene()
{}

BaseScene::~BaseScene()
{
    Client::get_instance()->clearOnMessageEvents();
    delete _pInfoText;
}

void BaseScene::initBase()
{
    // Set clear color
    Application::get()->getMasterRenderer().setClearColor(Panel::get_base_ui_color(0));

    activeCamera = createEntity();
    Camera::create(activeCamera, { 0, 1.0f, 0 }, 0.0f, 0.0f);
    pk::Debug::log("@BaseScene::initBase Loading default font");
    pk::Application* pApp = pk::Application::get();
    pk::ResourceManager& resourceManager = pApp->getResourceManager();
    _pDefaultFont = resourceManager.createFont(
        "assets/fonts/Matamata-Regular.otf",
        20
    );
    _pSmallFont = resourceManager.createFont(
        "assets/fonts/Matamata-Regular.otf",
        18
    );

    // Create directional light
    directionalLight = createEntity();
    pk::vec3 lightDirection(0.33f, -0.33f, -0.33f);
    lightDirection.normalize();
    DirectionalLight::create(
        directionalLight,
        { 1.0f, 1.0f, 1.0f },
        lightDirection
    );

    _pInfoText = new GUIText(
        "", *_pDefaultFont,
        {
            HorizontalConstraintType::PIXEL_LEFT, 0,
            VerticalConstraintType::PIXEL_TOP, 0
        }
    );
}

void BaseScene::setInfoText(
    const std::string& txt,
    pk::vec3 color,
    float horizontalVal,
    float verticalVal,
    pk::HorizontalConstraintType horizontalConstraint,
    pk::VerticalConstraintType verticalConstraint
)
{
    ConstraintData* pConstraintData = _pInfoText->getConstraint();
    pConstraintData->horizontalType = horizontalConstraint;
    pConstraintData->horizontalValue = horizontalVal;
    pConstraintData->verticalType = verticalConstraint;
    pConstraintData->verticalValue = verticalVal;

    TextRenderable* pRenderable = _pInfoText->getRenderable();

    pRenderable->accessStr() = txt;
    pRenderable->color = color;
}
