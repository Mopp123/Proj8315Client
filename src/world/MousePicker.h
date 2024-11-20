#pragma once

#include "World.h"
#include "ppk.h"


namespace  world
{
    class MousePicker
    {
    private:
        pk::Scene* _pScene = nullptr;
        World* _pWorld = nullptr;;

        entityID_t _cursorEntity = 0;

        pk::vec3 _worldCoords;
        int _tileX = 0;
        int _tileY = 0;

        int _selectedTileX = 0;
        int _selectedTileY = 0;

        uint64_t _selectedTile = 0;

    public:
        MousePicker() = default;
        void init(pk::Scene* pScene, World* pWorld);
        void update(bool clampToTile);
        void set3DCursorVisible(bool arg);

        // Returns picked coords in "observed area space" where coords are
        // always between 0 to observeAreaWidth - 1
        void getPickedObserveSpaceCoords(int32_t& outX, int32_t& outY) const;
        void setSelectedTile(uint64_t tile, int tileX, int tileY);

        inline int getPickedTileX() const { return _tileX; }
        inline int getPickedTileY() const { return _tileY; }
        inline int getSelectedTileX() const { return _selectedTileX; }
        inline int getSelectedTileY() const { return _selectedTileY; }

        inline uint64_t getSelectedTile() const { return _selectedTile; }
    };
}
