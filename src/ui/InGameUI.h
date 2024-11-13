#pragma once

#include "Panel.h"
#include "net/Client.h"
#include <vector>
#include <string>
#include "SelectedPanel.h"
#include "TileOptionsMenu.h"


class InGame;

class InGameUI
{
private:
    class OnClickLogout : public pk::ui::OnClickEvent
    {
    private:
        InGame* _pInGameScene = nullptr;

    public:
        OnClickLogout(InGame* pInGameScene) :_pInGameScene(pInGameScene) {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnMessageLogout : public net::OnMessageEvent
    {
    public:
        OnMessageLogout() {}
        virtual void onMessage(const GC_byte* data, size_t dataSize);
    };

    InGame* _pInGameScene = nullptr;
    // Just quick hack to be able to use "InGameLocal" scene for testing..
    pk::Scene* _pScene = nullptr;

    // Panel containing stuff like, logout, settings/preferences, etc..
    // Couldn't fugure out better name..
    Panel _settingsPanel;

    // Contains info about currently selected tile and/or object.
    // TODO:
    // * If selected object -> this should follows object from tile
    // to another eventually!
    SelectedPanel _selectedPanel;
    // That "right click dropdown menu"
    TileOptionsMenu _tileOptionsMenu;

    // Panel what u can do when right clicking on a tile/object
    Panel _actionsPanel;

public:
    InGameUI() {};
    ~InGameUI() {};

    // NOTE:
    // *If testing locally give pInGameScene as nullptr
    // *pScene is required to be the current scene casted to "raw scene"
    void create(InGame* pInGameScene, pk::Scene* pScene, pk::Font* pFont);

    inline SelectedPanel& getSelectedPanel() { return _selectedPanel; }
    inline TileOptionsMenu& getTileOptionsMenu() { return _tileOptionsMenu; }
};
