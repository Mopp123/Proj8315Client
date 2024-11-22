#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "core/input/InputEvent.h"
#include "ecs/components/ui/ConstraintData.h"
#include "ecs/factories/ui/UIFactories.h"


#define UI_BASE_COLOR_COUNT 4

class Panel
{
public:
    enum LayoutFillType
    {
        VERTICAL = 0,
        HORIZONTAL = 1
    };

    class PanelCursorPosEvent : public pk::CursorPosEvent
    {
    private:
        pk::Scene* _pScene = nullptr;
        Panel* _pPanel = nullptr;

    public:
        PanelCursorPosEvent(pk::Scene* pScene, Panel* pPanel) :
            _pScene(pScene),
            _pPanel(pPanel)
        {}
		virtual void func(int x, int y);
    };

protected:
    friend class PanelCursorPosEvent;

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

    bool _isMouseOver = false;

    static pk::vec4* s_uiColor;

    static int s_pickedPanels;

public:
    Panel() {};
    virtual ~Panel() {};

    void create(
        pk::Scene* pScene,
        pk::Font* pDefaultFont,
        pk::HorizontalConstraintType horizontalType, float horizontalValue,
        pk::VerticalConstraintType verticalType, float verticalValue,
        pk::vec2 scale,
        LayoutFillType fillType = LayoutFillType::VERTICAL,
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

    pk::ui::UIFactoryButton addDefaultButton(
        std::string txt,
        pk::ui::OnClickEvent* onClick,
        float width
    );

    pk::ui::UIFactoryButton addButton(
        std::string txt,
        pk::ui::OnClickEvent* onClick,
        pk::HorizontalConstraintProperties horizontalConstraint,
        pk::VerticalConstraintProperties verticalConstraint,
        pk::vec2 scale,
        bool drawBorder = false
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

    void setLayer(int layer);

    // Returns current up to date pos and scale of the panel
    pk::Rect2D getRect() const; // :D

    static pk::vec4 get_base_ui_color(unsigned int colorIndex);

    static bool is_mouse_over_ui();

    inline bool isMouseOver() const { return _isMouseOver; }
    inline entityID_t getEntity() const { return _entity; }

private:
    pk::vec2 calcNewSlotPos();
};
