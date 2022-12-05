#include "ui.h"


using namespace pk;
using namespace ui;


Panel::Panel(
    std::pair<pk::ui::ConstraintType, float> horizontalConstraint,
    std::pair<pk::ui::ConstraintType, float> verticalConstraint,
    vec2 slotScale,
    PanelLayoutType layoutType
) :
    _horizontalConstraint(horizontalConstraint),
    _verticalConstraint(verticalConstraint),
    _slotScale(slotScale),
    _layoutType(layoutType)
{}

Panel::~Panel()
{}

void Panel::addText(std::string txt)
{
    vec2 toAdd = calcNewSlotPos();
    create_text(
        txt,
        _horizontalConstraint.first, 
        _horizontalConstraint.second + toAdd.y,
        _verticalConstraint.first, 
        _verticalConstraint.second + toAdd.y);
   _slotCount++;
}

void Panel::addButton(std::string txt, pk::ui::OnClickEvent* onClick)
{
    vec2 toAdd = calcNewSlotPos();
    create_button(
        txt,
        _horizontalConstraint.first, 
        _horizontalConstraint.second + toAdd.x,
        _verticalConstraint.first, 
        _verticalConstraint.second + toAdd.y,
        _slotScale.x, _slotScale.y,
        onClick
    );
    _slotCount++;
}

vec2 Panel::calcNewSlotPos()
{
    if (_layoutType == PanelLayoutType::VERTICAL)
        return { 0.0f,  (_slotScale.y + _slotPadding) * _slotCount};
    else if (_layoutType == PanelLayoutType::HORIZONTAL)
        return { (_slotScale.x + _slotPadding) * _slotCount, 0.0f };
}
