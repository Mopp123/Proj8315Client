#pragma once

#include "Panel.h"
#include "ecs/components/ui/ConstraintData.h"
#include "ecs/factories/ui/UIFactories.h"

class TopBarPanel : public Panel
{
protected:
    class OnClickClose : public pk::ui::OnClickEvent
    {
    private:
        TopBarPanel* _pPanel = nullptr;
    public:
        OnClickClose(TopBarPanel* pPanel) : _pPanel(pPanel) {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    entityID_t _topBarImgEntity = 0;
    entityID_t _topBarTitleEntity = 0;
    pk::ui::UIFactoryButton _topBarCloseButton;

public:
    void initBase(
        pk::Scene* pScene,
        pk::Font* pFont,
        const std::string title,
        pk::ConstraintProperties constraintProperties,
        const pk::vec2& scale
    );

    virtual void open() = 0;
    virtual void close() = 0;

protected:
    void setComponentsActive(bool arg);
};
