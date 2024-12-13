#pragma once

#include "pesukarhu/ppk.h"
#include <vector>


// For displaying info about currently selected tile/object
class SelectedPanel : public pk::ui::Panel
{
private:
    enum TileInfoSlotIndex
    {
        type = 0,
        elevation,
        temperature,
        effect,
        position
    };

    const int _portraitTextureRows = 16;

    pk::ui::GUIText* _pObjectName = 0;

    std::vector<pk::ui::GUIText*> _statusInfoEntities;
    std::vector<pk::ui::GUIText*> _attributeInfoEntities;
    std::vector<pk::ui::GUIText*> _tileInfoEntities;

    pk::ui::GUIImage* _pSelectedPortraitImg = nullptr;

public:
    SelectedPanel() {};
    ~SelectedPanel() {};

    // Not sure how to handle create funcs... can't/shouldn't really call these
    // the same as in the original Panel class...
    void init(pk::Scene* pScene, pk::Font* pFont);

    void setSelectedInfo(uint64_t tile, int tileX, int tileY);

private:
    // Returns created info txt entities (doesn't include the title)
    std::vector<pk::ui::GUIText*> addInfoColumn(
        int columnIndex,
        float columnWidth,
        const pk::vec2& pos,
        float txtPaddingY,
        float txtHeight,
        const std::string& title,
        const std::vector<std::string>& infos
    );
};
