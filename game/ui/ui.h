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
    pk::vec2 _slotScale = { 100, 24 };
    int _slotCount = 0;
    float _slotPadding = 1.0f;

    std::pair<pk::ui::ConstraintType, float> _horizontalConstraint;
    std::pair<pk::ui::ConstraintType, float> _verticalConstraint;

    PanelLayoutType _layoutType;

public:
    Panel(
        std::pair<pk::ui::ConstraintType, float> horizontalConstraint,
        std::pair<pk::ui::ConstraintType, float> verticalConstraint,
        pk::vec2 slotScale = pk::vec2(100,24),
        PanelLayoutType layoutType = PanelLayoutType::VERTICAL
    );
    ~Panel();

    void addText(std::string txt);
    void addButton(std::string txt, pk::ui::OnClickEvent* onClick);

private:
    pk::vec2 calcNewSlotPos();
};
