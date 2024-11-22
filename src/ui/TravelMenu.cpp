#include "TravelMenu.h"
#include "ecs/factories/ui/UIFactories.h"
#include <stdexcept>


using namespace pk;
using namespace pk::ui;


void TravelMenu::OnClickTravel::onClick(pk::InputMouseButtonName button)
{
    pMenu->travel();
}


void TravelMenu::init(pk::Scene* pScene, pk::Font* pFont)
{
    vec2 scale(172, 87);
    initBase(
        pScene,
        pFont,
        "Enter location",
        {
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            -scale.x * 0.5f,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL,
            scale.y * 0.5f,
        },
        scale
    );
    _layoutType = LayoutFillType::VERTICAL;

    const float inputFieldWidth = 60.0f;
    const float inputFieldY = scale.y * 0.5f - _slotScale.y - _slotPadding;
    _inputFieldEntityX = addInputField(
        "x",
        {
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            -scale.x * 0.5f + _slotPadding,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL,
            inputFieldY
        },
        inputFieldWidth,
        nullptr
    ).contentEntity;

    _inputFieldEntityZ = addInputField(
        "z",
        {
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            -scale.x * 0.5f + inputFieldWidth * 1.5f,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL,
            inputFieldY
        },
        inputFieldWidth,
        nullptr
    ).contentEntity;

    const float buttonWidth = 70.0f;
    addButton(
        "Travel",
        new OnClickTravel(this),
        {
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            -scale.x * 0.5f + _slotPadding,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL,
            inputFieldY - _slotScale.y - _slotPadding
        },
        { buttonWidth, _slotScale.y }
    );

    close();
}

void TravelMenu::open()
{
    setComponentsActive(true);
}

void TravelMenu::close()
{
    setComponentsActive(false);
}

void TravelMenu::setTargetCoords(int32_t targetX, int32_t targetZ)
{
    setInputFieldContents(std::to_string(targetX), std::to_string(targetZ));
}

void TravelMenu::travel()
{
    std::string xStr = "";
    std::string zStr = "";
    getInputFieldContents(xStr, zStr);
    int32_t x = 0;
    int32_t z = 0;
    try
    {
        x = std::stoi(xStr);
        z = std::stoi(zStr);
    }
    catch(const std::invalid_argument& e)
    {
        std::string exceptionStr(e.what());
        Debug::log(
            "@TravelMenu::travel "
            "Failed to convert input fields to valid coordinates. "
            "Invalid argument: " + exceptionStr,
            Debug::MessageType::PK_ERROR
        );
        return;
    }
    Debug::log("___TEST___travelling to: " + std::to_string(x) + ", " + std::to_string(z));
}

void TravelMenu::setInputFieldContents(const std::string& targetX, const std::string& targetZ)
{
    TextRenderable* pRenderableTargetX = (TextRenderable*)_pScene->getComponent(
        _inputFieldEntityX,
        ComponentType::PK_RENDERABLE_TEXT
    );
    TextRenderable* pRenderableTargetZ = (TextRenderable*)_pScene->getComponent(
        _inputFieldEntityZ,
        ComponentType::PK_RENDERABLE_TEXT
    );
    if (pRenderableTargetX)
        pRenderableTargetX->accessStr() = targetX;
    if (pRenderableTargetZ)
        pRenderableTargetZ->accessStr() = targetZ;
}

void TravelMenu::getInputFieldContents(std::string& outTargetX, std::string& outTargetZ) const
{
    const TextRenderable* pRenderableTargetX = (const TextRenderable*)_pScene->getComponent(
        _inputFieldEntityX,
        ComponentType::PK_RENDERABLE_TEXT
    );
    const TextRenderable* pRenderableTargetZ = (const TextRenderable*)_pScene->getComponent(
        _inputFieldEntityZ,
        ComponentType::PK_RENDERABLE_TEXT
    );
    if (pRenderableTargetX)
        outTargetX = pRenderableTargetX->getStr();
    if (pRenderableTargetZ)
        outTargetZ = pRenderableTargetZ->getStr();
}
