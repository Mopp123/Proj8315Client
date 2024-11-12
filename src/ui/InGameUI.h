#pragma once

#include "Panel.h"
#include "net/Client.h"
#include <vector>
#include <string>


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

    enum TileInfoSlotIndex
    {
        type = 0,
        elevation,
        temperature,
        effect,
        position
    };

    const int _portraitTextureRows = 16;

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
    Panel _selectedPanel;

    // Panel what u can do when right clicking on a tile/object
    Panel _actionsPanel;

    entityID_t _objectNameEntity = 0;
    std::vector<entityID_t> _statusInfoEntities;
    std::vector<entityID_t> _attributeInfoEntities;
    std::vector<entityID_t> _tileInfoEntities;

    entityID_t _selectedPortraitEntity = 0;

public:
    InGameUI() {};
    ~InGameUI() {};

    // NOTE:
    // *If testing locally give pInGameScene as nullptr
    // *pScene is required to be the current scene casted to "raw scene"
    void create(InGame* pInGameScene, pk::Scene* pScene, pk::Font* pFont);

    void setSelectedInfo(uint64_t tile, int tileX, int tileY);

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
