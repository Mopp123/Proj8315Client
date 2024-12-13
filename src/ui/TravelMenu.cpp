#include "TravelMenu.h"
#include "pesukarhu/ppk.h"
#include <stdexcept>


using namespace pk;
using namespace pk::ui;


void TravelMenu::OnClickTravel::onClick(pk::InputMouseButtonName button)
{
    pMenu->travel();
}


void TravelMenu::init(
    pk::Scene* pScene,
    world::World* pWorld,
    CameraController* pCamController,
    pk::Font* pFont
)
{
    _pWorld = pWorld;
    _pCamController = pCamController;

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
        scale,
        LayoutFillType::VERTICAL
    );

    const float inputFieldWidth = 60.0f;
    const float inputFieldY = scale.y * 0.5f - _slotScale.y - _slotPadding;
    _pInputFieldX = addInputField(
        "x",
        {
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            -scale.x * 0.5f + _slotPadding,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL,
            inputFieldY
        },
        inputFieldWidth,
        nullptr
    );

    _pInputFieldZ = addInputField(
        "z",
        {
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            -scale.x * 0.5f + inputFieldWidth * 1.5f,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL,
            inputFieldY
        },
        inputFieldWidth,
        nullptr
    );

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
    _pInputFieldX->setContent(std::to_string(targetX));
    _pInputFieldZ->setContent(std::to_string(targetZ));
}

void TravelMenu::travel()
{
    std::string xStr = _pInputFieldX->getContent();
    std::string zStr = _pInputFieldZ->getContent();
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

    const float tileVisualScale = _pWorld->getTileVisualScale();
    _pCamController->setPivotPoint({ (float)x * tileVisualScale, 0, (float)z * tileVisualScale });
    close();
}
