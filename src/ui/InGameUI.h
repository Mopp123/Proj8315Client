#pragma once

#include "Pesukarhu/ppk.h"
#include "net/Client.h"
#include <vector>
#include <string>
#include "SelectedPanel.h"
#include "TileOptionsMenu.h"
#include "scenes/BaseScene.h"
#include "world/World.h"
#include "CameraUtils.h"


class InGame;

class InGameUI
{
private:
    class OnClickLogout : public pk::ui::OnClickEvent
    {
    public:
        BaseScene* pScene = nullptr;
        OnClickLogout(BaseScene* pScene) : pScene(pScene) {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnMessageLogout : public net::OnMessageEvent
    {
    public:
        OnMessageLogout() {}
        virtual void onMessage(const GC_byte* data, size_t dataSize);
    };

    BaseScene* _pScene = nullptr;

    // Panel containing stuff like, logout, settings/preferences, etc..
    // Couldn't fugure out better name..
    pk::ui::Panel _settingsPanel;

    // Contains info about currently selected tile and/or object.
    // TODO:
    // * If selected object -> this should follows object from tile
    // to another eventually!
    SelectedPanel _selectedPanel;
    // That "right click dropdown menu"
    TileOptionsMenu _tileOptionsMenu;

    // Panel what u can do when right clicking on a tile/object
    pk::ui::Panel _actionsPanel;

public:
    InGameUI() {};
    ~InGameUI() {};

    // NOTE:
    // *If testing locally give pInGameScene as nullptr
    // *pScene is required to be the current scene casted to "raw scene"
    void create(
        BaseScene* pScene,
        world::World* pWorld,
        CameraController* pCamController,
        pk::Font* pFont,
        pk::Font* pSmallFont
    );

    inline SelectedPanel& getSelectedPanel() { return _selectedPanel; }
    inline TileOptionsMenu& getTileOptionsMenu() { return _tileOptionsMenu; }
};
