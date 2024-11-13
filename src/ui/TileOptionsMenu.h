#pragma once

#include "Panel.h"


class TileOptionsMenu : public Panel
{
private:
    const size_t _maxSlotItems = 10;

    std::vector<entityID_t> _slotEntities;

public:
    void init(pk::Scene* pScene, pk::Font* pFont);

    void show(uint64_t tileData, float x, float y);
    void hide();
};
