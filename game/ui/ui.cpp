#include "ui.h"


using namespace pk;
using namespace ui;


void Panel::init(
    std::pair<pk::ui::ConstraintType, float> horizontalConstraint,
    std::pair<pk::ui::ConstraintType, float> verticalConstraint,
    vec2 slotScale,
    bool drawBackground, pk::vec2 backgroundScale,
    PanelLayoutType layoutType
)
{
    _horizontalConstraint = horizontalConstraint;
    _verticalConstraint = verticalConstraint;
    _slotScale = slotScale;
    _layoutType = layoutType;

    _pCurrentScene = Application::get()->accessCurrentScene();
    _entity = _pCurrentScene->createEntity();

    if (drawBackground)
    {
        uint32_t backgroundImg = 0;
        backgroundImg = create_image(
            horizontalConstraint.first, horizontalConstraint.second - _slotPadding,
            verticalConstraint.first, verticalConstraint.second + _slotPadding,
            backgroundScale.x, backgroundScale.y,
            false,
            nullptr,
            { 0, 0, 1, 1},
            { 0.1f, 0.1f, 0.1f }
        );
        if (backgroundImg)
            _pCurrentScene->addChild(_entity, backgroundImg);
    }
}

Panel::~Panel()
{}

std::pair<uint32_t, TextRenderable*> Panel::addText(std::string txt)
{
    float charSize = (32.0f * 0.5f) * 0.5f;
    vec2 toAdd = calcNewSlotPos();
    std::pair<uint32_t, TextRenderable*> txtObj = create_text(
        txt,
        _horizontalConstraint.first,
        _horizontalConstraint.second + toAdd.x,
        _verticalConstraint.first,
        _verticalConstraint.second + toAdd.y
    );
    _pCurrentScene->addChild(_entity, txtObj.first);
    _slotCount++;
    return txtObj;
}

uint32_t Panel::addButton(
        std::string txt,
        pk::ui::OnClickEvent* onClick,
        bool selectable,
        pk::Texture* texture,
        pk::vec4 textureCropping,
        pk::vec3 color
)
{
    vec2 toAdd = calcNewSlotPos();
    uint32_t buttonEntity = create_button(
        txt,
        _horizontalConstraint.first,
        _horizontalConstraint.second + toAdd.x,
        _verticalConstraint.first,
        _verticalConstraint.second + toAdd.y,
        _slotScale.x, _slotScale.y,
        onClick,
        selectable,
        texture,
        textureCropping,
        color
    );
    _pCurrentScene->addChild(_entity, buttonEntity);
    _slotCount++;
    return buttonEntity;
}

std::pair<uint32_t, pk::TextRenderable*> Panel::addInputField(
    std::string infoTxt,
    float width,
    pk::ui::InputFieldOnSubmitEvent* onSubmit,
    bool clearOnSubmit
)
{
    vec2 toAdd = calcNewSlotPos();
    std::pair<uint32_t, TextRenderable*> inputField = create_input_field(
        infoTxt,
        _horizontalConstraint.first,
        _horizontalConstraint.second + toAdd.x,
        _verticalConstraint.first,
        _verticalConstraint.second + toAdd.y,
        width,
        onSubmit,
        clearOnSubmit
    );
    _pCurrentScene->addChild(_entity, inputField.first);
    _slotCount++;
    return inputField;
}

void Panel::setActive(bool arg)
{
    Scene* scene = Application::get()->accessCurrentScene();
    int count = 0;
    for (Component* c : scene->getAllComponents(_entity))
    {
        c->setActive(arg);
        count++;
    }
}

vec2 Panel::calcNewSlotPos()
{
    vec2 pos(0, 0);
    if (_layoutType == PanelLayoutType::VERTICAL)
        pos = { 0.0f,  (_slotScale.y + _slotPadding) * _slotCount};
    else if (_layoutType == PanelLayoutType::HORIZONTAL)
        pos = { (_slotScale.x + _slotPadding) * _slotCount, 0.0f };

    if (_verticalConstraint.first == ConstraintType::PIXEL_CENTER_VERTICAL)
        pos.y *= -1.0f;

    return pos;
}
