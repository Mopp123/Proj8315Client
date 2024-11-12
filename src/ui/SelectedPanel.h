#pragma once

#include "Panel.h"
#include <vector>


// For displaying info about currently selected tile/object
class SelectedPanel : public Panel
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

    entityID_t _objectNameEntity = 0;

    std::vector<entityID_t> _statusInfoEntities;
    std::vector<entityID_t> _attributeInfoEntities;
    std::vector<entityID_t> _tileInfoEntities;

    entityID_t _selectedPortraitEntity = 0;

public:
    SelectedPanel() {};
    ~SelectedPanel() {};

    // Not sure how to handle create funcs... can't/shouldn't really call these
    // the same as in the original Panel class...
    void init(pk::Scene* pScene, pk::Font* pFont);

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
