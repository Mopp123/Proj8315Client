#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "ecs/components/ui/ConstraintData.h"


class Panel
{
public:
    enum LayoutFillType
    {
        VERTICAL = 0,
        HORIZONTAL = 1
    };

private:
    pk::Scene* _pScene = nullptr;
    entityID_t _entity;


    pk::HorizontalConstraintType _horizontalConstraint;
    float _horizontalConstraintValue;
    pk::VerticalConstraintType _verticalConstraint;
    float _verticalConstraintValue;

    LayoutFillType _layoutType; // how added buttons, etc. are placed to panel
    pk::vec2 _scale;

    float _slotPadding = 1.0f;
    pk::vec2 _slotScale = pk::vec2(200.0f, 60.0f);
    pk::vec3 _color;
    pk::vec4 _borderColor;

    int _slotCount = 0;

public:
    Panel() {};
    ~Panel() {};

    void create(
        pk::Scene* pScene,
        pk::HorizontalConstraintType horizontalType, float horizontalValue,
        pk::VerticalConstraintType verticalType, float verticalValue,
        pk::vec2 scale,
        LayoutFillType fillType = LayoutFillType::VERTICAL,
        bool drawBackground = false,
        pk::vec3 color = pk::vec3(0, 0, 0),
        bool drawBorder = false,
        pk::vec4 borderColor = pk::vec4(0, 0, 0, 0), // dont remember why this was vec4 on Scene.create
        float borderThickness = 2.0f,
        float slotPadding = 1.0f
    );

    void addButton(
        std::string txt,
        pk::ui::OnClickEvent* onClick,
        bool selectable,
        pk::vec3 color,
        pk::Font& font
    );

private:
    pk::vec2 calcNewSlotPos();
};
