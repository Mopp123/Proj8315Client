#include "Panel.h"


using namespace pk;
using namespace pk::ui;


static vec4 s_defaultUIColorsLight0[] = {
    { 201, 197, 172, 255 },
    { 215, 209, 185, 255 },
    { 180, 173, 151, 255 },
    //{ 136, 128, 99, 255 }
    { 107, 102, 85, 255 }
};
static vec4 s_defaultUIColorsMonochrome[] = {
    { 187, 187, 187, 255 },
    { 200, 200, 200, 255 },
    { 166, 166, 166, 255 },
    { 96, 96, 96, 255 }
};
static vec4 s_defaultUIColorsUIDark[] = {
    { 28, 28, 28, 255 },
    { 58, 58, 58, 255 },
    { 48, 48, 48, 255 },
    { 188, 188, 188, 255 }
};
vec4* Panel::s_uiColor = s_defaultUIColorsUIDark;


int Panel::s_pickedPanels = 0;


void Panel::PanelCursorPosEvent::func(int x, int y)
{
    // check activeness by looking at "root entity's" transform's activeness
    // -> fucking stupid way of doing this but it'll do for now..
    Transform* pTransform = (Transform*)_pScene->getComponent(
        _pPanel->getEntity(),
        ComponentType::PK_TRANSFORM
    );
    if (!pTransform)
    {
        Debug::log(
            "@Panel::PanelCursorPosEvent::func "
            "Panel's root entity doesn't have valid Transform component!",
            Debug::MessageType::PK_FATAL_ERROR
        );
        return;
    }
    if (!pTransform->isActive())
    {
        _pPanel->_isMouseOver = false;
        return;
    }

    float fx = (float)x;
    float fy = (float)y;

    Rect2D panelRect = _pPanel->getRect();
    float panelX = panelRect.offsetX;
    float panelY = panelRect.offsetY;
    float panelWidth = panelRect.width;
    float panelHeight = panelRect.height;

    if (fx >= panelX && fx <= panelX + panelWidth && fy <= panelY && fy >= panelY - panelHeight)
    {
        _pPanel->_isMouseOver = true;
        ++s_pickedPanels;
    }
    else
    {
        _pPanel->_isMouseOver = false;
        if (s_pickedPanels - 1 >= 0)
            --s_pickedPanels;
    }
}


void Panel::create(
    Scene* pScene,
    Font* pDefaultFont,
    HorizontalConstraintType horizontalType, float horizontalValue,
    VerticalConstraintType verticalType, float verticalValue,
    vec2 scale,
    LayoutFillType layoutType,
    vec3 color,
    bool drawBorder,
    vec4 borderColor,
    float borderThickness,
    float slotPadding,
    vec2 slotScale
)
{
    _pScene = pScene;
    _pDefaultFont = pDefaultFont;

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
    _slotScale = slotScale;

    _entity = _pScene->createEntity();

    vec4 textureCropping(0, 0, 1, 1); // unused since no even texure atm?

    ImgCreationProperties imgCreationProperties;
    imgCreationProperties.constraintProperties = {
        horizontalType,
        horizontalValue,
        verticalType,
        verticalValue
    };
    imgCreationProperties.width = _scale.x;
    imgCreationProperties.height = _scale.y;
    imgCreationProperties.color = _color;
    imgCreationProperties.borderColor = _borderColor;
    imgCreationProperties.borderThickness = borderThickness;
    imgCreationProperties.textureCropping = textureCropping;

    _entity = create_image(imgCreationProperties);

    InputManager* pInputManager = Application::get()->accessInputManager();
    pInputManager->addCursorPosEvent(new PanelCursorPosEvent(pScene, this));
}

void Panel::createDefault(
    Scene* pScene,
    Font* pDefaultFont,
    HorizontalConstraintType horizontalType, float horizontalValue,
    VerticalConstraintType verticalType, float verticalValue,
    vec2 scale,
    vec2 slotScale,
    LayoutFillType fillType,
    int useColorIndex
)
{
    create(
        pScene,
        pDefaultFont,
        horizontalType, horizontalValue,
        verticalType, verticalValue,
        scale,
        fillType,
        get_base_ui_color(useColorIndex).toVec3(),
        false, // draw border
        { 0, 0, 0, 1 }, // border color
        0, // border thickness
        4.0f, // slot padding;
        slotScale
    );
}

std::pair<entityID_t, pk::TextRenderable*> Panel::addText(
    std::string txt,
    pk::HorizontalConstraintType horizontalType, float horizontalValue,
    pk::VerticalConstraintType verticalType, float verticalValue
)
{
    const vec2 offsetFromPanel(4.0f, 4.0f);
    std::pair<entityID_t, TextRenderable*> text = pk::ui::create_text(
        txt, *_pDefaultFont,
        horizontalType,
        horizontalValue,
        verticalType,
        verticalValue,
        get_base_ui_color(3).toVec3(),
        false // bold
    );
    _pScene->addChild(_entity, text.first);
    ++_slotCount;

    return text;
}

std::pair<entityID_t, TextRenderable*> Panel::addText(std::string txt, vec3 color)
{
    const vec2 offsetFromPanel(4.0f, 4.0f);
    vec2 toAdd = calcNewSlotPos();
    std::pair<entityID_t, TextRenderable*> text = pk::ui::create_text(
        txt, *_pDefaultFont,
        _horizontalConstraint,
        _horizontalConstraintValue + toAdd.x + offsetFromPanel.x,
        _verticalConstraint,
        _verticalConstraintValue + toAdd.y - offsetFromPanel.y,
        color,
        false // bold
    );
    _pScene->addChild(_entity, text.first);
    ++_slotCount;

    return text;
}

std::pair<entityID_t, TextRenderable*> Panel::addDefaultText(std::string txt)
{
    return addText(txt, get_base_ui_color(3).toVec3());
}

UIFactoryButton Panel::addDefaultButton(
    std::string txt,
    ui::OnClickEvent* onClick,
    float width
)
{
    vec4 color = get_base_ui_color(2);
    vec4 borderColor = color;

    const float borderThickness = 0.0f;
    Texture* pTexture = nullptr;
    vec4 textureCropping(0, 0, 1, 1);

    vec2 offsetFromPanel(4.0f, 4.0f);
    // offsetting depends which constraint type we are using
    if (_verticalConstraint == VerticalConstraintType::PIXEL_TOP)
        offsetFromPanel.y *= -1.0f;

    vec2 toAdd = calcNewSlotPos();
    UIFactoryButton button = create_button(
        txt,
        *_pDefaultFont,
        _horizontalConstraint,
        _horizontalConstraintValue + toAdd.x + offsetFromPanel.x,
        _verticalConstraint,
        _verticalConstraintValue + toAdd.y - offsetFromPanel.y,
        width, _slotScale.y, // scale
        onClick,
        false,
        color.toVec3(), // color
        get_base_ui_color(3).toVec3(), // text color
        get_base_ui_color(1).toVec3(), // text highlight color
        get_base_ui_color(3).toVec3(), // background highlight color
        color, // border color
        borderThickness,
        pTexture,
        textureCropping
    );
    // atm fucks up because constraint and transform systems are in conflict?
    _pScene->addChild(_entity, button.rootEntity);
    ++_slotCount;
    return button;
}

pk::ui::UIFactoryButton Panel::addButton(
    std::string txt,
    OnClickEvent* onClick,
    ConstraintProperties constraintProperties,
    vec2 scale,
    bool drawBorder
)
{
    vec4 color = get_base_ui_color(2);
    vec4 borderColor = color;
    float borderThickness = 0.0f;
    if (drawBorder)
    {
        borderColor = get_base_ui_color(3);
        borderThickness = 1.0f;
    }

    Texture* pTexture = nullptr;
    vec4 textureCropping(0, 0, 1, 1);

    UIFactoryButton button = create_button(
        txt,
        *_pDefaultFont,
        constraintProperties.horizontalType,
        constraintProperties.horizontalValue,
        constraintProperties.verticalType,
        constraintProperties.verticalValue,
        scale.x, scale.y,
        onClick,
        false,
        color.toVec3(), // color
        get_base_ui_color(3).toVec3(), // text color
        get_base_ui_color(1).toVec3(), // text highlight color
        get_base_ui_color(3).toVec3(), // background highlight color
        borderColor, // border color
        borderThickness,
        pTexture,
        textureCropping
    );
    // atm fucks up because constraint and transform systems are in conflict?
    _pScene->addChild(_entity, button.rootEntity);
    // Atm disabling adding to slot count since this overrides the "slot" thing completely...
    //++_slotCount;
    return button;
}

UIFactoryInputField Panel::addDefaultInputField(
    std::string infoTxt,
    int width,
    pk::ui::InputFieldOnSubmitEvent* onSubmitEvent,
    bool clearOnSubmit,
    bool password
)
{
    vec4 color = get_base_ui_color(2);
    const vec2 offsetFromPanel(4.0f, 4.0f);
    vec2 toAdd = calcNewSlotPos();
    UIFactoryInputField inputField = ui::create_input_field(
        infoTxt, *_pDefaultFont,
        _horizontalConstraint,
        _horizontalConstraintValue + toAdd.x + offsetFromPanel.x,
        _verticalConstraint,
        _verticalConstraintValue + toAdd.y - offsetFromPanel.y,
        width,
        nullptr, // on submit event
        false, // clear on submit
        color.toVec3(), // color
        get_base_ui_color(3).toVec3(), // text color
        get_base_ui_color(3).toVec3(), // text highlight color
        get_base_ui_color(1).toVec3(), // background highlight color,
        password
    );
    _pScene->addChild(_entity, inputField.rootEntity);
    ++_slotCount;

    return inputField;
}

UIFactoryInputField Panel::addInputField(
    std::string infoTxt,
    pk::ConstraintProperties constraintProperties,
    int width,
    pk::ui::InputFieldOnSubmitEvent* onSubmitEvent,
    bool clearOnSubmit,
    bool password
)
{
    vec4 color = get_base_ui_color(2);
    UIFactoryInputField inputField = ui::create_input_field(
        infoTxt, *_pDefaultFont,
        constraintProperties.horizontalType,
        constraintProperties.horizontalValue,
        constraintProperties.verticalType,
        constraintProperties.verticalValue,
        width,
        nullptr, // on submit event
        false, // clear on submit
        color.toVec3(), // color
        get_base_ui_color(3).toVec3(), // text color
        get_base_ui_color(3).toVec3(), // text highlight color
        get_base_ui_color(1).toVec3(), // background highlight color
        password
    );
    _pScene->addChild(_entity, inputField.rootEntity);

    return inputField;
}

entityID_t Panel::addImage(
    pk::HorizontalConstraintType horizontalType, float horizontalVal,
    pk::VerticalConstraintType verticalType, float verticalVal,
    float width, float height,
    pk::Texture* pTexture,
    pk::vec3 color,
    pk::vec4 textureCropping
)
{
    entityID_t imgEntity = ui::create_image(
        horizontalType, horizontalVal,
        verticalType, verticalVal,
        width, height,
        color, // color
        { color.x, color.y, color.z, 1.0f }, // border color
        0.0f, // border thickness
        pTexture,
        textureCropping
    );
    // NOTE: Earlier this img wasn't added as child... don't remember was there
    // a reason for it...
    _pScene->addChild(_entity, imgEntity);
    // NOTE: Not sure should _slotCount increase when adding img...
    return imgEntity;
}

void Panel::setActive(bool arg, entityID_t entity)
{
    if (entity == 0)
        entity = _entity;

    std::vector<Component*> components = _pScene->getComponents(entity);
    for (Component* pComponent : components)
        pComponent->setActive(arg);
    std::vector<entityID_t> children = _pScene->getChildren(entity);
    for (entityID_t child: children)
        setActive(arg, child);
}

void Panel::setLayer(int layer)
{
    for (Component* pComponent : _pScene->getAllComponents(_entity))
    {
        ComponentType type = pComponent->getType();
        if (type == ComponentType::PK_RENDERABLE_GUI)
        {
            GUIRenderable* pRenderable = (GUIRenderable*)pComponent;
            pRenderable->setLayer(layer);
        }
        else if (type == ComponentType::PK_RENDERABLE_TEXT)
        {
            TextRenderable* pRenderable = (TextRenderable*)pComponent;
            pRenderable->setLayer(layer);
        }
    }
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

Rect2D Panel::getRect() const
{
    const Transform* pTransform = (const Transform*)_pScene->getComponent(
        _entity,
        ComponentType::PK_TRANSFORM
    );
    const mat4& tMat = pTransform->getTransformationMatrix();
    int32_t x = tMat[0 + 3 * 4];
    int32_t y = tMat[1 + 3 * 4];
    uint32_t width = tMat[0 + 0 * 4];
    uint32_t height = tMat[1 + 1 * 4];
    return { x, y, width, height};
}

vec4 Panel::get_base_ui_color(unsigned int colorIndex)
{
    if (colorIndex >= UI_BASE_COLOR_COUNT)
    {
        Debug::log(
            "@Panel::get_base_ui_color "
            "Invalid color index: " + std::to_string(colorIndex) + " "
            "Last available color index is " + std::to_string(UI_BASE_COLOR_COUNT - 1),
            Debug::MessageType::PK_FATAL_ERROR
        );
        return { 0, 0, 0, 1 };
    }
    vec4 color = s_uiColor[colorIndex];
    color.x /= 255;
    color.y /= 255;
    color.z /= 255;
    color.w /= 255;
    return color;
}

bool Panel::is_mouse_over_ui()
{
    return s_pickedPanels > 0;
}
