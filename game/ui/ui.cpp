#include "ui.h"


using namespace pk;
using namespace ui;


Panel::Panel(
    std::pair<pk::ui::ConstraintType, float> horizontalConstraint,
    std::pair<pk::ui::ConstraintType, float> verticalConstraint,
    vec2 slotScale,
    bool drawBackground, pk::vec2 backgroundScale,
    PanelLayoutType layoutType
) :
    _horizontalConstraint(horizontalConstraint),
    _verticalConstraint(verticalConstraint),
    _slotScale(slotScale),
    _layoutType(layoutType)
{
    _pCurrentScene = Application::get()->accessCurrentScene();
    _entity = _pCurrentScene->createEntity();

    if (drawBackground)
    {
        // NOTE: Not configured for other than vertical center constraint!!!
        uint32_t backgroundImg = create_image(
            horizontalConstraint.first, horizontalConstraint.second,
            verticalConstraint.first, verticalConstraint.second - backgroundScale.y * 0.5f,
            backgroundScale.x, backgroundScale.y,
            false,
            { 0.1f, 0.1f, 0.1f }
        );
        _pCurrentScene->addChild(_entity, backgroundImg);
    }
}

Panel::~Panel()
{}

void Panel::addText(std::string txt)
{
    float charSize = (32.0f * 0.5f) * 0.5f;
    vec2 toAdd = calcNewSlotPos();
    uint32_t txtEntity = create_text(
        txt,
        _horizontalConstraint.first, 
        _horizontalConstraint.second + toAdd.x - txt.size() * charSize,
        _verticalConstraint.first, 
        _verticalConstraint.second + toAdd.y
    ).first;
    _pCurrentScene->addChild(_entity, txtEntity);
   _slotCount++;
}

void Panel::addButton(std::string txt, pk::ui::OnClickEvent* onClick)
{
    vec2 toAdd = calcNewSlotPos();
    uint32_t buttonEntity = create_button(
        txt,
        _horizontalConstraint.first, 
        _horizontalConstraint.second + toAdd.x,
        _verticalConstraint.first, 
        _verticalConstraint.second + toAdd.y,
        _slotScale.x, _slotScale.y,
        onClick
    );
    _pCurrentScene->addChild(_entity, buttonEntity);
    _slotCount++;
}

void Panel::addInputField(
    std::string infoTxt,
    float width,
    pk::ui::InputFieldOnSubmitEvent* onSubmit,
    bool clearOnSubmit
)
{
    vec2 toAdd = calcNewSlotPos();
    uint32_t inputFieldEntity = create_input_field(
        infoTxt,
        _horizontalConstraint.first, 
        _horizontalConstraint.second + toAdd.x,
        _verticalConstraint.first, 
        _verticalConstraint.second + toAdd.y,
        width,
        onSubmit,
        clearOnSubmit
    ).first;
    _pCurrentScene->addChild(_entity, inputFieldEntity);
    _slotCount++;
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
    Debug::log("___TEST___FOUND COUNT = " + std::to_string(count));
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
