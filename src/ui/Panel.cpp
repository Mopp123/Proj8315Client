#include "Panel.h"

using namespace pk;


void Panel::create(
    pk::Scene* pScene,
    pk::HorizontalConstraintType horizontalType, float horizontalValue,
    pk::VerticalConstraintType verticalType, float verticalValue,
    vec2 scale,
    LayoutFillType layoutType,
    bool drawBackground,
    pk::vec3 color,
    bool drawBorder,
    pk::vec4 borderColor,
    float borderThickness,
    float slotPadding
)
{
    _pScene = pScene;

    _horizontalConstraint = horizontalType;
    _horizontalConstraintValue = horizontalValue;
    _verticalConstraint = verticalType;
    _verticalConstraintValue = verticalValue;

    _scale = scale;
    _layoutType = layoutType;

    _color = color;
    if (drawBorder)
        _borderColor = borderColor;
    else
        _borderColor = { _color.x, _color.y, _color.z, 1.0f };

    _slotPadding = slotPadding;

    _entity = _pScene->createEntity();
    // may not be needed..
    vec2 pos(0, 0);


    Transform* pTransform = _pScene->createTransform(
        _entity,
        pos,
        _scale
    );

    ConstraintData* pConstraint = _pScene->createUIConstraint(
        _entity,
        horizontalType,
        horizontalValue,
        verticalType,
        verticalValue
    );

    GUIRenderable* pBackground = nullptr;
    Texture_new* pBackgroundTexture = nullptr; // unused atm
    vec4 textureCropping(0, 0, 1, 1);
    if (drawBackground)
    {
        pBackground = _pScene->createGUIRenderable(
            _entity,
            pBackgroundTexture,
            _color,
            _borderColor,
            borderThickness,
            textureCropping
        );
    }
}

void Panel::addButton(
    std::string txt,
    pk::ui::OnClickEvent* onClick,
    bool selectable,
    pk::vec3 color,
    pk::Font& font
)
{
    vec4 borderColor(0.6f, 0.6f, 0.6f, 1.0f);
    float borderThickness = 2.0f;
    Texture_new* pTexture = nullptr;
    vec4 textureCropping(0, 0, 1, 1);

    vec2 toAdd = calcNewSlotPos();
    uint32_t buttonEntity = create_button(
        txt, font,
        _horizontalConstraint,
        _horizontalConstraintValue + toAdd.x,
        _verticalConstraint,
        0,
        //_verticalConstraintValue + toAdd.y,
        _slotScale.x, _slotScale.y,
        onClick,
        selectable,
        color,
        borderColor,
        borderThickness,
        pTexture,
        textureCropping
    );
    // atm fucks up because constraint and transform systems are in conflict?
    _pScene->addChild(_entity, buttonEntity);
    ++_slotCount;

    /*
    // TODO: figure these out
    float width = 200.0f;
    float height = 55.0f;

    entityID_t buttonEntity = create_button(
        txt, font,
        _horizontalConstraint, _horizontalConstraintValue + xPos,
        _verticalConstraint, _verticalConstraintValue + yPos,
        float width, float height,
        OnClickEvent* onClick,
        bool selectable = false,
        vec3 color = { 0.1f, 0.1f, 0.1f },
        vec4 borderColor = { 0.6f, 0.6f, 0.6f, 1.0f },
        float borderThickness = 2,
        Texture_new* pTexture = nullptr,
        vec4 textureCropping = vec4(0, 0, 1, 1)
    );
    */
}

vec2 Panel::calcNewSlotPos()
{
    vec2 pos(0, 0);
    if (_layoutType == LayoutFillType::VERTICAL)
        pos = { 0.0f,  (_slotScale.y + _slotPadding) * _slotCount};
    else if (_layoutType == LayoutFillType::HORIZONTAL)
        pos = { (_slotScale.x + _slotPadding) * _slotCount, 0.0f };

    if (_verticalConstraint == VerticalConstraintType::PIXEL_CENTER_VERTICAL)
        pos.y *= -1.0f;

    return pos;
}
