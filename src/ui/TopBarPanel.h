#pragma once

#include "Panel.h"
#include "ecs/factories/ui/UIFactories.h"

class TopBarPanel : public Panel
{
protected:
    entityID_t _topBarImgEntity = 0;
    entityID_t _topBarTitleEntity = 0;
    pk::ui::UIFactoryButton _topBarCloseButton;

public:
    void initBase(
        pk::Scene* pScene,
        pk::Font* pFont,
        const std::string title,
        pk::HorizontalConstraintProperties horizontalConstraint,
        pk::VerticalConstraintProperties verticalConstraint,
        const pk::vec2& scale,
        pk::ui::OnClickEvent* pCloseButtonOnClick
    );

protected:
    void setComponentsActive(bool arg);
};
