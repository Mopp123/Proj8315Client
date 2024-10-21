#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "ecs/components/ui/ConstraintData.h"


#define UI_BASE_COLOR_COUNT 4

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
    pk::Font* _pDefaultFont = nullptr;
    entityID_t _entity;


    pk::HorizontalConstraintType _horizontalConstraint;
    float _horizontalConstraintValue;
    pk::VerticalConstraintType _verticalConstraint;
    float _verticalConstraintValue;

    LayoutFillType _layoutType; // how added buttons, etc. are placed to panel
    pk::vec2 _scale;

    float _slotPadding = 1.0f;
    pk::vec2 _slotScale;
    pk::vec3 _color;
    pk::vec4 _borderColor;

    int _slotCount = 0;

    static pk::vec4* s_uiColor;

public:
    Panel() {};
    ~Panel() {};

    void create(
        pk::Scene* pScene,
        pk::Font* pDefaultFont,
        pk::HorizontalConstraintType horizontalType, float horizontalValue,
        pk::VerticalConstraintType verticalType, float verticalValue,
        pk::vec2 scale,
        LayoutFillType fillType = LayoutFillType::VERTICAL,
        bool drawBackground = false,
        pk::vec3 color = pk::vec3(0, 0, 0),
        bool drawBorder = false,
        pk::vec4 borderColor = pk::vec4(0, 0, 0, 0), // dont remember why this was vec4 on Scene.create
        float borderThickness = 2.0f,
        float slotPadding = 1.0f,
        pk::vec2 slotScale = pk::vec2(200.0f, 24.0f)
    );

    void createDefault(
        pk::Scene* pScene,
        pk::Font* pDefaultFont,
        pk::HorizontalConstraintType horizontalType, float horizontalValue,
        pk::VerticalConstraintType verticalType, float verticalValue,
        pk::vec2 scale,
        pk::vec2 slotScale,
        LayoutFillType fillType = LayoutFillType::VERTICAL,
        int useColorIndex = 1
    );

    void createDefault(
        pk::Scene* pScene,
        pk::Font* pDefaultFont,
        pk::HorizontalConstraintType horizontalType, float horizontalValue,
        pk::VerticalConstraintType verticalType, float verticalValue,
        pk::vec2 scale,
        pk::vec2 slotScale,
        LayoutFillType fillType,
        pk::vec3 color
    );

    std::pair<entityID_t, pk::TextRenderable*> addText(
        std::string txt,
        pk::HorizontalConstraintType horizontalType, float horizontalValue,
        pk::VerticalConstraintType verticalType, float verticalValue
    );
    std::pair<entityID_t, pk::TextRenderable*> addText(std::string txt, pk::vec3 color);
    std::pair<entityID_t, pk::TextRenderable*> addDefaultText(std::string txt);

    void addDefaultButton(
        std::string txt,
        pk::ui::OnClickEvent* onClick,
        float width
    );

    std::pair<entityID_t, pk::TextRenderable*> addDefaultInputField(
        std::string infoTxt,
        int width,
        pk::ui::InputFieldOnSubmitEvent* onSubmitEvent,
        bool clearOnSubmit = false
    );

    entityID_t addImage(
        pk::HorizontalConstraintType horizontalType, float horizontalVal,
        pk::VerticalConstraintType verticalType, float verticalVal,
        float width, float height,
        pk::Texture* pTexture,
        pk::vec3 color,
        pk::vec4 textureCropping = pk::vec4(0, 0, 1, 1)
    );

    void setActive(bool arg, entityID_t entity = 0);

    static pk::vec4 get_base_ui_color(unsigned int colorIndex);

private:
    pk::vec2 calcNewSlotPos();
};
