#pragma once

#include "pesukarhu/ppk.h"
#include "../../Proj8315Common/src/Object.h"
#include <vector>


class SpawnMenu : public pk::ui::TopBarPanel
{
private:
    class MenuItemOnClick : public pk::ui::GUIButton::OnClickEvent
    {
    private:
        SpawnMenu* _pMenu = nullptr;
        int _itemIndex = 0;
    public:
        MenuItemOnClick(SpawnMenu* pMenu, int menuItemIndex) :
            _pMenu(pMenu),
            _itemIndex(menuItemIndex)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    size_t _maxSpawnButtons = 0;
    std::vector<pk::ui::GUIButton*> _spawnSelectionButtonEntities;

    uint64_t _selectedTileData = 0;
    int32_t _selectedTileX = 0;
    int32_t _selectedTileY = 0;

public:
    void init(pk::Scene* pScene, pk::Font* pFont);

    virtual void open();
    virtual void close();

    void setSelectedTile(uint64_t tileData, int32_t x, int32_t y);

private:
    void displaySpawnButtons(const std::vector<gamecommon::ObjectInfo>& objects);
};
