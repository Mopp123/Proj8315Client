#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "BaseScene.h"
#include "../ui/ui.h"

// Declared here because we need two way link between CurrentRosterPanel and SelectedObjPanel
class SelectedObjPanel;


typedef struct RosterObject
{
    uint32_t entity = 0;
    PK_ubyte objType = 0;
    int objCount = 0;
    pk::GUIRenderable* pImgRenderable = nullptr;
    pk::TextRenderable* pTxtRenderable = nullptr;
    void setActive(bool arg)
    {
        if (pImgRenderable)
            pImgRenderable->setActive(arg);
        if (pTxtRenderable)
            pTxtRenderable->setActive(arg);
    }
} RosterObject;


class CurrentRosterPanel
{
private:
    pk::TextRenderable* _title;
    Panel _panel;

    static int s_slotCount;

public:
    std::vector<RosterObject> rosterObjects;

    void init(
        std::pair<pk::ui::ConstraintType, float> horizontalConstraint,
        std::pair<pk::ui::ConstraintType, float> verticalConstraint,
        pk::vec2 rosterButtonScale
    );
    // NOTE: This NEEDS TO BE CALLED after SelectedObjPanel has been created
    // since this requires access to SelectedObjPanel!
    void initSlotButtons(SelectedObjPanel& selectedObjPanelRef);
    RosterObject* getAvailableSlot(PK_ubyte objType);
};

class SelectedObjPanel
{
private:
    CurrentRosterPanel* _pCurrentRosterPanel = nullptr;
    pk::TextRenderable* _title;
    pk::TextRenderable* _description;
    std::vector<pk::TextRenderable*> _statsInfo;
    Panel _panel;
    std::vector<pk::Component*> _buttonComponents;

    PK_ubyte _selectedObj = 0;

public:
    void init(
        CurrentRosterPanel* pCurrentRosterPanel,
        std::pair<pk::ui::ConstraintType, float> horizontalConstraint,
        std::pair<pk::ui::ConstraintType, float> verticalConstraint
    );
    void hideButtons();
    void showButtons();
    void setSelected(PK_ubyte objType);
    inline PK_ubyte getSelected() const { return _selectedObj; }
};


class RosterSelectionPanel
{
private:
    SelectedObjPanel* _pInfoPanel;
    pk::TextRenderable* _title;
    Panel _panel;

    static int s_slotDisplayCount;

public:
    void init(
        SelectedObjPanel* _pInfoPanel,
        std::string titleStr,
        std::pair<pk::ui::ConstraintType, float> horizontalConstraint,
        std::pair<pk::ui::ConstraintType, float> verticalConstraint,
        pk::vec2 rosterButtonScale
    );
    void addSlot(PK_ubyte objType);
};


class DeploymentMenu : public BaseScene
{
private:
    CurrentRosterPanel _currentRosterPanel;
    SelectedObjPanel _selectedObjPanel;

    RosterSelectionPanel _personnelPanel;
    RosterSelectionPanel _cargoPanel;

    Panel _detailsDescriptionPanel;
    Panel _detailsStatsPanel;

public:
    DeploymentMenu();
    ~DeploymentMenu();

    virtual void init();
    virtual void update();
};
