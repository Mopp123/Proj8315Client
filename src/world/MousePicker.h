#pragma once

#include "World.h"


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

    public:

        MousePicker() = default;

        void init(pk::Scene* pScene, World* pWorld);

        void update(bool clampToTile);

        void set3DCursorVisible(bool arg);
    };
}
