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


void TopBarPanel::initBase(
    pk::Scene* pScene,
    pk::Font* pFont,
    const std::string title,
    HorizontalConstraintProperties horizontalConstraint,
    VerticalConstraintProperties verticalConstraint,
    const vec2& scale,
    OnClickEvent* pCloseButtonOnClick
)
{
    vec2 slotScale(200, 24);
    createDefault(
        pScene,
        pFont,
        horizontalConstraint.type, horizontalConstraint.value,
        verticalConstraint.type, verticalConstraint.value,
        scale,
        slotScale,
        Panel::LayoutFillType::HORIZONTAL,
        0
    );

    const float topBarHeight = 20.0f;
    // Create top bar (atm just an img)
    // TODO: Maybe in the future make top bar as "button" which u can drag the panel around
    _topBarImgEntity = addImage(
        horizontalConstraint.type, horizontalConstraint.value,
        verticalConstraint.type, verticalConstraint.value,
        scale.x, topBarHeight,
        nullptr, // texture
        Panel::get_base_ui_color(2).toVec3(),
        { 0, 0, 1, 1 } // texture cropping
    );
    // Add title text
    _topBarTitleEntity = addText(
        title,
        horizontalConstraint.type, horizontalConstraint.value,
        verticalConstraint.type, verticalConstraint.value
    ).first;

    HorizontalConstraintProperties closeButtonHorizontalConstraint =
    {
        horizontalConstraint.type,
        horizontalConstraint.value + scale.x - topBarHeight
    };
    // Add close button
    _topBarCloseButton = addButton(
        "X",
        pCloseButtonOnClick,
        closeButtonHorizontalConstraint,
        verticalConstraint,
        { topBarHeight - 1, topBarHeight - 1 }, // scale
        true
    );
}

void TopBarPanel::setComponentsActive(bool arg)
{
    for(Component* pComponent : _pScene->getComponents(_entity))
        pComponent->setActive(arg);
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
}
