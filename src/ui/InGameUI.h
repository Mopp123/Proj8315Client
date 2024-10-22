#pragma once

#include "Panel.h"
#include <vector>
#include <string>


class InGameUI
{
private:
    pk::Scene* _pScene = nullptr;

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

    entityID_t _objectNameEntity = 0;
    std::vector<entityID_t> _statusInfoEntities;
    std::vector<entityID_t> _attributeInfoEntities;
    std::vector<entityID_t> _tileInfoEntities;

public:
    InGameUI() {};
    ~InGameUI() {};

    void create(pk::Scene* pScene, pk::Font* pFont);

    void setSelectedInfo(uint64_t tile);

private:
    // Returns created info txt entities (doesn't include the title)
    std::vector<entityID_t> addInfoColumn(
        int columnIndex,
        float columnWidth,
        const pk::vec2& pos,
        float txtPaddingY,
        float txtHeight,
        const std::string& title,
        const std::vector<std::string>& infos
    );
};
