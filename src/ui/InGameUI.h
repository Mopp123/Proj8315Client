#pragma once

#include "Panel.h"


class InGameUI
{
private:
    // Panel containing stuff like, logout, settings/preferences, etc..
    // Couldn't fugure out better name..
    Panel _settingsPanel;

    // Contains info about currently selected tile and/or object.
    // TODO:
    // * If selected object -> this should follows object from tile
    // to another eventually!
    Panel _selectedPanel;

    // Panel what u can do when right clicking on a tile/object
    Panel _actionsPanel;

public:
    InGameUI() {};
    ~InGameUI() {};

    void create(pk::Scene* pScene, pk::Font* pFont);
};
