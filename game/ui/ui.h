#pragma once

#include <utility>
#include <string>
#include "../../PortablePesukarhu/ppk.h"

enum PanelLayoutType{
    VERTICAL = 0,
    HORIZONTAL = 1
};


class Panel
{
private:
    uint32_t _entity = 0;
    pk::Scene* _pCurrentScene = nullptr;
    pk::vec2 _slotScale = { 100, 24 };
    int _slotCount = 0;
    float _slotPadding = 4.0f;

    std::pair<pk::ui::ConstraintType, float> _horizontalConstraint;
    std::pair<pk::ui::ConstraintType, float> _verticalConstraint;

    PanelLayoutType _layoutType;

    std::vector<pk::Component*> _allComponents;

public:
    Panel(
        std::pair<pk::ui::ConstraintType, float> horizontalConstraint,
        std::pair<pk::ui::ConstraintType, float> verticalConstraint,
        pk::vec2 slotScale = pk::vec2(100,24),
        bool drawBackground = false, pk::vec2 backgroundScale = { 0, 0 },
        PanelLayoutType layoutType = PanelLayoutType::VERTICAL
    );
    ~Panel();

    void addText(std::string txt);
    void addButton(std::string txt, pk::ui::OnClickEvent* onClick);
    void addInputField(
        std::string infoTxt,
        float width,
        pk::ui::InputFieldOnSubmitEvent* onSubmit,
        bool clearOnSubmit = false
    );

    void setActive(bool arg);

private:
    pk::vec2 calcNewSlotPos();
};
