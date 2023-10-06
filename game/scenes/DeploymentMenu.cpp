#include "DeploymentMenu.h"
#include "MainMenu.h"
#include "../world/Objects.h"
#include "InGame.h"
#include "../../Proj8315Common/src/Object.h"


using namespace pk;
using namespace web;
using namespace ui;
using namespace net;
using namespace gamecommon;


int CurrentRosterPanel::s_slotCount = 8;
int RosterSelectionPanel::s_slotDisplayCount = 5;


class OnClickSelectObj : public OnClickEvent
{
public:
    PK_ubyte objType = 0;
    SelectedObjPanel& infoPanelRef;

    OnClickSelectObj(
        SelectedObjPanel& infoPanelRef,
        PK_ubyte objType
    ) :
        infoPanelRef(infoPanelRef),
        objType(objType)
    {}

    virtual void onClick(InputMouseButtonName button)
    {
        infoPanelRef.setSelected(objType);
    }
};


class OnClickSelectObjFromRoster : public OnClickEvent
{
public:
    CurrentRosterPanel& rosterPanelRef;
    SelectedObjPanel& infoPanelRef;
    int slotIndex;

    OnClickSelectObjFromRoster(
        CurrentRosterPanel& rosterPanelRef,
        SelectedObjPanel& infoPanelRef,
        int slotIndex
    ) :
        rosterPanelRef(rosterPanelRef),
        infoPanelRef(infoPanelRef),
        slotIndex(slotIndex)
    {}

    virtual void onClick(InputMouseButtonName button)
    {
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        {
            infoPanelRef.setSelected(rosterPanelRef.rosterObjects[slotIndex].objType);
        }
    }
};


class OnClickAddToRoster : public OnClickEvent
{
public:
    CurrentRosterPanel& rosterPanelRef;
    SelectedObjPanel& infoPanelRef;

    OnClickAddToRoster(
        CurrentRosterPanel& rosterPanelRef,
        SelectedObjPanel& infoPanelRef
    ):
        rosterPanelRef(rosterPanelRef),
        infoPanelRef(infoPanelRef)
    {}

    virtual void onClick(InputMouseButtonName button)
    {
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        {
            PK_ubyte objType = infoPanelRef.getSelected();
            ObjectInfo* obj = world::objects::ObjectInfoLib::get(objType);
            world::objects::VisualObjectInfo* visualObj = world::objects::ObjectInfoLib::getVisual(objType);
            if (!obj)
            {
                Debug::log(
                    "@OnClickAddToRoster::onClick(InputMouseButtonName)\n"
                    "   ObjInfo for: " + std::to_string(objType) + " doesnt exist!",
                    Debug::MessageType::PK_ERROR
                );
                return;
            }

            RosterObject* slot = rosterPanelRef.getAvailableSlot(objType);
            if (slot)
            {
                if (!slot->pImgRenderable || !slot->pTxtRenderable)
                {
                    Debug::log(
                        "@OnClickAddToRoster::onClick(InputMouseButtonName)\n"
                        "   Couldn't find renderable components for slot of type: " + std::to_string(objType),
                        Debug::MessageType::PK_ERROR
                    );
                    return;
                }
                slot->objType = objType;
                slot->objCount++;
                slot->pImgRenderable->texture = (Texture*)visualObj->pTexture;
                slot->pTxtRenderable->accessStr() = std::to_string(slot->objCount);
                slot->setActive(true);
            }
        }
    }
};


class OnClickRemoveFromRoster : public OnClickEvent
{
public:
    CurrentRosterPanel& rosterPanelRef;
    SelectedObjPanel& infoPanelRef;

    OnClickRemoveFromRoster(
        CurrentRosterPanel& rosterPanelRef,
        SelectedObjPanel& infoPanelRef
    ):
        rosterPanelRef(rosterPanelRef),
        infoPanelRef(infoPanelRef)
    {}

    virtual void onClick(InputMouseButtonName button)
    {
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        {
            PK_ubyte objType = infoPanelRef.getSelected();
            RosterObject* slot = rosterPanelRef.getAvailableSlot(objType);
            if (slot)
            {
                if (slot->objType != 0)
                {
                    if (!slot->pImgRenderable || !slot->pTxtRenderable)
                    {
                        Debug::log(
                            "@OnClickRemoveFromRoster::onClick(InputMouseButtonName)\n"
                            "   Couldn't find renderable components for slot of type: " + std::to_string(objType),
                            Debug::MessageType::PK_ERROR
                        );
                        return;
                    }
                    slot->objCount--;
                    slot->pTxtRenderable->accessStr() = std::to_string(slot->objCount);
                    if (slot->objCount <= 0)
                    {
                        slot->objType = 0;
                        slot->pTxtRenderable->accessStr() = "";
                        slot->pImgRenderable->texture = nullptr;
                        slot->setActive(false);
                    }
                }
            }
        }
    }
};


void CurrentRosterPanel::init(
    std::pair<pk::ui::ConstraintType, float> horizontalConstraint,
    std::pair<pk::ui::ConstraintType, float> verticalConstraint,
    vec2 rosterButtonScale
)
{
    _title = create_text(
        "To Deploy",
        horizontalConstraint.first, horizontalConstraint.second,
        verticalConstraint.first, verticalConstraint.second + 20
    ).second;

    _panel.init(
        horizontalConstraint,
        verticalConstraint,
        rosterButtonScale,
        true,
        { rosterButtonScale.x * s_slotCount,  rosterButtonScale.y},
        PanelLayoutType::HORIZONTAL
    );
}

void CurrentRosterPanel::initSlotButtons(SelectedObjPanel& selectedObjPanelRef)
{
    Scene* scene = Application::get()->accessCurrentScene();

    // world::objects::VisualObject::TexturePortraitCropping portraitCropping = world::objects::VisualObject::s_defaultPortraitCropping;

    world::objects::VisualObjectInfo::TexturePortraitCropping portraitCropping = world::objects::VisualObjectInfo::s_defaultPortraitCropping;
    for (int i = 0; i < s_slotCount; ++i)
    {
        uint32_t entity = _panel.addButton(
            "",
            new OnClickSelectObjFromRoster(*this, selectedObjPanelRef, i),
            false,
            nullptr,
            { portraitCropping.pos.x, portraitCropping.pos.y, portraitCropping.scale.x, portraitCropping.scale.y },
            { 0.8f, 0.8f, 0.8f }
        );
        GUIRenderable* pImgRenderable = (GUIRenderable*)scene->getComponentInChildren(entity, ComponentType::PK_RENDERABLE_GUI);
        TextRenderable* pTxtRenderable = (TextRenderable*)scene->getComponentInChildren(entity, ComponentType::PK_RENDERABLE_TEXT);
        rosterObjects.push_back({ entity, 0, 0 , pImgRenderable, pTxtRenderable});
        rosterObjects[i].setActive(false);
    }
}

RosterObject* CurrentRosterPanel::getAvailableSlot(PK_ubyte objType)
{
    ObjectInfo* obj = world::objects::ObjectInfoLib::get(objType);
    if (obj)
    {
        RosterObject* slot = nullptr;
        // first search if theres already slots for this type
        for (int i = 0; i < rosterObjects.size(); ++i)
        {
            RosterObject* objSlot = &rosterObjects[i];
            if (objSlot->objType == objType)
            {
                Debug::log("___TEST___FOUND EXISTING SLOT!");
                return objSlot;
            }
        }
        // if not search for empty slot
        for (int i = 0; i < rosterObjects.size(); ++i)
        {
            RosterObject* objSlot = &rosterObjects[i];
            if (objSlot->objType == 0)
            {
                Debug::log("___TEST___FOUND NEW EMPTY SLOT!");
                return objSlot;
            }
        }
    }
    return nullptr;
}


void SelectedObjPanel::init(
    CurrentRosterPanel* pCurrentRosterPanel,
    std::pair<pk::ui::ConstraintType, float> horizontalConstraint,
    std::pair<pk::ui::ConstraintType, float> verticalConstraint
)
{
    _pCurrentRosterPanel = pCurrentRosterPanel;
    _title = create_text(
        "",
        horizontalConstraint.first, horizontalConstraint.second,
        verticalConstraint.first, verticalConstraint.second + 20
    ).second;

    _panel.init(
        horizontalConstraint,
        verticalConstraint,
        { 100, 30 },
        true,
        { 285, 220 },
        PanelLayoutType::VERTICAL
    );

    _description = _panel.addText("Select cargo and personnel\nto recruit").second;
    uint32_t addButton = _panel.addButton("add", new OnClickAddToRoster(*pCurrentRosterPanel, *this));
    uint32_t removeButton = _panel.addButton("remove", new OnClickRemoveFromRoster(*pCurrentRosterPanel, *this));

    // Find and hide add/remove buttons initially
    Scene* scene = Application::get()->accessCurrentScene();
    for (Component* component : scene->getComponentsInChildren(addButton))
        _buttonComponents.push_back(component);
    for (Component* component : scene->getComponentsInChildren(removeButton))
        _buttonComponents.push_back(component);
    hideButtons();
}

void SelectedObjPanel::hideButtons()
{
    for (Component* component: _buttonComponents)
        component->setActive(false);
}

void SelectedObjPanel::showButtons()
{
    for (Component* component: _buttonComponents)
        component->setActive(true);
}

void SelectedObjPanel::setSelected(PK_ubyte objType)
{
    if (objType != 0)
    {
        ObjectInfo* obj = world::objects::ObjectInfoLib::get(objType);
        if (obj)
        {
            // Initially show buttons but after that -> unnecessary to set visible again..
            if (!_buttonComponents.empty())
            {
                showButtons();
                _buttonComponents.clear();
            }
            _selectedObj = objType;
            _title->accessStr() = std::string(obj->name);
            _description->accessStr() = std::string(obj->description);
        }
    }
}


void RosterSelectionPanel::init(
    SelectedObjPanel* pInfoPanel,
    std::string titleStr,
    std::pair<pk::ui::ConstraintType, float> horizontalConstraint,
    std::pair<pk::ui::ConstraintType, float> verticalConstraint,
    vec2 rosterButtonScale
)
{
    _pInfoPanel = pInfoPanel;

    _title = create_text(
        titleStr,
        horizontalConstraint.first, horizontalConstraint.second,
        verticalConstraint.first, verticalConstraint.second + 20
    ).second;

    _panel.init(
        horizontalConstraint,
        verticalConstraint,
        rosterButtonScale,
        false,
        { 0, 0 },
        PanelLayoutType::HORIZONTAL
    );
}

// TODO: Better error handling if no objType is found!
void RosterSelectionPanel::addSlot(PK_ubyte objType)
{
    ObjectInfo* obj = world::objects::ObjectInfoLib::get(objType);
    if (!obj)
    {
        Debug::log("@RosterSelectionPanel::addSlot(PK_ubyte objType) object not found", Debug::MessageType::PK_FATAL_ERROR);
        return;
    }
    world::objects::VisualObjectInfo* visualObj = world::objects::ObjectInfoLib::getVisual(objType);
    WebTexture* texture = nullptr;
    if (!visualObj)
    {
        Debug::log("@RosterSelectionPanel::addSlot(PK_ubyte objType) visual object not found", Debug::MessageType::PK_FATAL_ERROR);
        return;
    }
    texture = visualObj->pTexture;

    const world::objects::VisualObjectInfo::TexturePortraitCropping& portraitCropping = visualObj->portraitCropping;
    _panel.addButton(
        std::to_string((int)objType),
        new OnClickSelectObj(*_pInfoPanel, objType),
        false,
        texture,
        { portraitCropping.pos.x, portraitCropping.pos.y, portraitCropping.scale.x, portraitCropping.scale.y },
        { 0.8f, 0.8f, 0.8f }
    );
}


class OnClickBack : public OnClickEvent
{
public:
    OnClickBack()
    {}

    virtual void onClick(InputMouseButtonName button)
    {
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        {
            Application::get()->switchScene((Scene*)(new MainMenu));
        }
    }
};


// TODO: Go to in game "arrival state",  using this
class OnClickDeploy : public OnClickEvent
{
public:
    CurrentRosterPanel& rosterPanelRef;

    OnClickDeploy(CurrentRosterPanel& rosterPanelRef) :
        rosterPanelRef(rosterPanelRef)
    {}

    virtual void onClick(InputMouseButtonName button)
    {
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        {
            Debug::log("___TEST___SWITCHING TO IN GAME!");
            Application::get()->switchScene((Scene*)(new InGame));
        }
    }
};


DeploymentMenu::DeploymentMenu()
{}

DeploymentMenu::~DeploymentMenu()
{}

void DeploymentMenu::init()
{
    ((BaseScene*)this)->initBase();

    vec2 buttonScale(100, 24);
    vec2 rosterButtonScale(60, 100);

    _currentRosterPanel.init(
        { ConstraintType::PIXEL_CENTER_HORIZONTAL, -300 },
        { ConstraintType::PIXEL_CENTER_VERTICAL, -150 },
        rosterButtonScale
    );

    vec2 detailsPanelScale(235, 120);
    vec2 detailsPanelSlotScale(60, 100);
    _selectedObjPanel.init(
        &_currentRosterPanel,
        { ConstraintType::PIXEL_CENTER_HORIZONTAL, 10 },
        { ConstraintType::PIXEL_CENTER_VERTICAL, detailsPanelScale.y + 11 }
    );

    _currentRosterPanel.initSlotButtons(_selectedObjPanel);

    int rosterSlotDisplayCount = 5;
    vec2 personnelPanelPos(
        -50 - (rosterButtonScale.x * rosterSlotDisplayCount),
        rosterButtonScale.y + 35
    );
    vec2 cargoPanelPos(
        -50 - (rosterButtonScale.x * rosterSlotDisplayCount),
        personnelPanelPos.y - rosterButtonScale.y * 1.25f
    );

    _personnelPanel.init(
        &_selectedObjPanel,
        "Personnel",
        { ConstraintType::PIXEL_CENTER_HORIZONTAL, personnelPanelPos.x },
        { ConstraintType::PIXEL_CENTER_VERTICAL, personnelPanelPos.y },
        rosterButtonScale
    );
    // test texturing roster slots
    _personnelPanel.addSlot(1);
    _personnelPanel.addSlot(2);
    for (int i = 2; i < rosterSlotDisplayCount; ++i)
        _personnelPanel.addSlot(0);

    _cargoPanel.init(
        &_selectedObjPanel,
        "Cargo",
        { ConstraintType::PIXEL_CENTER_HORIZONTAL, cargoPanelPos.x },
        { ConstraintType::PIXEL_CENTER_VERTICAL, cargoPanelPos.y },
        rosterButtonScale
    );
    for (int i = 0; i < rosterSlotDisplayCount; ++i)
        _cargoPanel.addSlot(0);

    create_button(
        "Back",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, -220 - buttonScale.x * 0.5f,
        ConstraintType::PIXEL_CENTER_VERTICAL, -280,
        buttonScale.x, buttonScale.y,
        new OnClickBack
    );

    create_button(
        "Deploy",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, 220 - buttonScale.x * 0.5f,
        ConstraintType::PIXEL_CENTER_VERTICAL, -280,
        buttonScale.x, buttonScale.y,
        new OnClickDeploy(_currentRosterPanel)
    );

    /*
    _detailsDescriptionPanel.init(
        { ConstraintType::PIXEL_CENTER_HORIZONTAL, 10 },
        { ConstraintType::PIXEL_CENTER_VERTICAL, detailsPanelScale.y + 32 },
        rosterButtonScale,
        true,
        detailsPanelScale,
        PanelLayoutType::HORIZONTAL
    );
    _detailsDescriptionPanel.addText("img");
    _detailsDescriptionPanel.addText("description");
    _detailsStatsPanel.init(
        { ConstraintType::PIXEL_CENTER_HORIZONTAL, 10 },
        { ConstraintType::PIXEL_CENTER_VERTICAL, detailsPanelScale.y + 32 - detailsPanelScale.y },
        { 16, 16 },
        true,
        detailsPanelScale,
        PanelLayoutType::VERTICAL
    );
    _detailsStatsPanel.addText("stat 1");
    _detailsStatsPanel.addText("stat 2");
    _detailsStatsPanel.addText("stat 3");
    _detailsStatsPanel.addText("stat 4");
    _detailsStatsPanel.addText("stat 5");
    */
}

void DeploymentMenu::update()
{}
