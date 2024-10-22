#pragma once

#include "World.h"
#include "ppk.h"


namespace  world
{
    class MousePicker
    {
    private:
        class PickerMouseButtonEvent : public pk::MouseButtonEvent
        {
        private:
            MousePicker* _pMousePicker = nullptr;
            PK_ubyte _clickState = 0;
        public:
            PickerMouseButtonEvent(MousePicker* pMousePicker) :
                _pMousePicker(pMousePicker)
            {}
		    virtual void func(pk::InputMouseButtonName button, pk::InputAction action, int mods);
        };

        friend class PickerMouseButtonEvent;

        pk::Scene* _pScene = nullptr;
        World* _pWorld = nullptr;;

        entityID_t _cursorEntity = 0;

        pk::vec3 _worldCoords;
        int _tileX = 0;
        int _tileY = 0;

        uint64_t _selectedTile = 0;

    public:
        MousePicker() = default;
        void init(pk::Scene* pScene, World* pWorld);
        void update(bool clampToTile);
        void set3DCursorVisible(bool arg);

        // Returns picked coords in "observed area space" where coords are
        // always between 0 to observeAreaWidth - 1
        void getPickedObserveSpaceCoords(int32_t& outX, int32_t& outY) const;

        inline void setSelectedTile(uint64_t tile) { _selectedTile = tile; }
        inline uint64_t getSelectedTile() const { return _selectedTile; }
    };
}
