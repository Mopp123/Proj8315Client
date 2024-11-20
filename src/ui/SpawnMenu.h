#pragma once

#include "TopBarPanel.h"
#include "ecs/factories/ui/UIFactories.h"
#include "../../Proj8315Common/src/Object.h"
#include <vector>


class SpawnMenu : public TopBarPanel
{
private:
    class MenuItemOnClick : public pk::ui::OnClickEvent
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

    const size_t _maxSpawnButtons = 20;
    std::vector<pk::ui::UIFactoryButton> _spawnSelectionButtonEntities;

    int32_t _selectedTileX = 0;
    int32_t _selectedTileY = 0;

public:
    void init(pk::Scene* pScene, pk::Font* pFont);

    void open(uint64_t tileData, int32_t selectedTileX, int32_t selectedTileY);
    virtual void close();

private:
    void displaySpawnButtons(const std::vector<gamecommon::ObjectInfo>& objects);
};
