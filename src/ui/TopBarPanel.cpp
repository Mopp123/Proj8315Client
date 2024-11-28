#include "TopBarPanel.h"
#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/Tile.h"
#include "Object.h"
#include "world/Objects.h"
#include "net/Client.h"


using namespace pk;
using namespace pk::ui;
using namespace world;
using namespace gamecommon;
using namespace net;


void TopBarPanel::OnClickClose::onClick(pk::InputMouseButtonName button)
{
    _pPanel->close();
}


void TopBarPanel::initBase(
    Scene* pScene,
    Font* pFont,
    const std::string title,
    ConstraintProperties constraintProperties,
    const vec2& scale
)
{
    vec2 slotScale(200, 24);
    createDefault(
        pScene,
        pFont,
        constraintProperties.horizontalType,
        constraintProperties.horizontalValue,
        constraintProperties.verticalType,
        constraintProperties.verticalValue,
        scale,
        slotScale,
        Panel::LayoutFillType::HORIZONTAL,
        0
    );

    const float topBarHeight = 20.0f;
    // Create top bar (atm just an img)
    // TODO: Maybe in the future make top bar as "button" which u can drag the panel around
    _topBarImgEntity = addImage(
        constraintProperties.horizontalType,
        constraintProperties.horizontalValue,
        constraintProperties.verticalType,
        constraintProperties.verticalValue,
        scale.x, topBarHeight,
        nullptr, // texture
        Panel::get_base_ui_color(2).toVec3(),
        { 0, 0, 1, 1 } // texture cropping
    );
    // Add title text
    _topBarTitleEntity = addText(
        title,
        constraintProperties.horizontalType,
        constraintProperties.horizontalValue,
        constraintProperties.verticalType,
        constraintProperties.verticalValue
    ).first;

    pScene->addChild(_entity, _topBarImgEntity);
    pScene->addChild(_entity, _topBarTitleEntity);

    ConstraintProperties closeButtonConstraintProperties =
    {
        constraintProperties.horizontalType,
        constraintProperties.horizontalValue + scale.x - topBarHeight,
        constraintProperties.verticalType,
        constraintProperties.verticalValue

    };
    // Add close button
    _topBarCloseButton = addButton(
        "X",
        new OnClickClose(this),
        closeButtonConstraintProperties,
        { topBarHeight - 1, topBarHeight - 1 }, // scale
        true
    );
}

void TopBarPanel::setComponentsActive(bool arg)
{
    for(Component* pComponent : _pScene->getAllComponents(_entity))
        pComponent->setActive(arg);

    /*
    for(Component* pComponent : _pScene->getComponents(_topBarImgEntity))
        pComponent->setActive(arg);
    for(Component* pComponent : _pScene->getComponents(_topBarTitleEntity))
        pComponent->setActive(arg);

    entityID_t closeButtonEntity = _topBarCloseButton.rootEntity;
    entityID_t closeButtonImgEntity = _topBarCloseButton.imgEntity;
    entityID_t closeButtonTxtEntity = _topBarCloseButton.txtEntity;
    for(Component* pComponent : _pScene->getComponents(closeButtonEntity))
        pComponent->setActive(arg);
    for(Component* pComponent : _pScene->getComponents(closeButtonImgEntity))
        pComponent->setActive(arg);
    for(Component* pComponent : _pScene->getComponents(closeButtonTxtEntity))
        pComponent->setActive(arg);
        */
}
