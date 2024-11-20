#pragma once

#include "Panel.h"
#include "core/input/InputEvent.h"
#include "ecs/factories/ui/UIFactories.h"
#include <vector>
#include "SpawnMenu.h"


class TileOptionsMenu;


struct DropDownMenuItem
{
    std::string txt;
    void (*func)(TileOptionsMenu*) = nullptr;
};


class TileOptionsMenu : public Panel
{
private:
    class MenuMouseButtonEvent : public pk::MouseButtonEvent
    {
    public:
        TileOptionsMenu* pMenu = nullptr;
        MenuMouseButtonEvent(TileOptionsMenu* pMenu) : pMenu(pMenu) {}
		virtual void func(pk::InputMouseButtonName button, pk::InputAction action, int mods);
    };

    class MenuItemOnClick : public pk::ui::OnClickEvent
    {
    private:
        TileOptionsMenu* _pMenu = nullptr;
        int _itemIndex = 0;
    public:
        MenuItemOnClick(TileOptionsMenu* pMenu, int menuItemIndex) :
            _pMenu(pMenu),
            _itemIndex(menuItemIndex)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    const size_t _maxMenuItems = 10;
    float _buttonTxtDisplacementX = 0.0f;

    std::vector<pk::ui::UIFactoryButton> _menuButtons;
    std::vector<DropDownMenuItem> _activeItems;

    // Need to have this here atm so we dont overlap with it...
    Panel* _pSelectedTilePanel = nullptr;

    uint64_t _selectedTile = 0;
    int _selectedTileX = 0;
    int _selectedTileY = 0;

    SpawnMenu _spawnMenu;

public:
    void init(pk::Scene* pScene, pk::Font* pFont, Panel* pSelectedTilePanel);

    void open(float screenX, float screenY, uint64_t tileData, int tileX, int tileY);
    void close();

    void openSpawnMenu();

private:
    void displayButton(float x, float y, int index, const std::string& txt);
    void setButtonActive(pk::ui::UIFactoryButton& button, bool arg);

    void updateActiveItemsList(uint64_t tile);
};
