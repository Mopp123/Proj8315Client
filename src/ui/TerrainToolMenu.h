#pragma once

#include "Tile.h"
#include "../../Proj8315Common/src/Object.h"
#include "pesukarhu/ppk.h"
#include <vector>


class TerrainToolMenu : public pk::ui::TopBarPanel
{
private:
    class TemperatureSelection : public TopBarPanel
    {
    public:
        TerrainToolMenu* pMenu = nullptr;
        void init(pk::Scene* pScene, pk::Font* pFont);
        virtual void open();
        virtual void close();
    };

    class TypeSelection : public TopBarPanel
    {
    public:
        TerrainToolMenu* pMenu = nullptr;
        void init(pk::Scene* pScene, pk::Font* pFont);
        virtual void open();
        virtual void close();
    };

    class SetValueEvent : public pk::ui::GUIButton::OnClickEvent
    {
    public:
        enum EventType
        {
            SET_TEMPERATURE,
            SET_TYPE
        };

        TerrainToolMenu* pToolMenu;
        TopBarPanel* pSelectionMenu;
        EventType type;
        GC_ubyte value;

        SetValueEvent(
            TerrainToolMenu* pToolMenu,
            TopBarPanel* pSelectionMenu,
            EventType eventType,
            GC_ubyte value
        ) :
            pToolMenu(pToolMenu),
            pSelectionMenu(pSelectionMenu),
            type(eventType),
            value(value)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OpenSelectionEvent : public pk::ui::GUIButton::OnClickEvent
    {
    public:
        enum class SelectionType
        {
            TEMPERATURE,
            TYPE
        };

        TerrainToolMenu* pMenu;
        SelectionType selection;

        OpenSelectionEvent(TerrainToolMenu* pMenu, SelectionType selection) :
            pMenu(pMenu),
            selection(selection)
        {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnClickApply : public pk::ui::GUIButton::OnClickEvent
    {
    public:
        TerrainToolMenu* pMenu;
        OnClickApply(TerrainToolMenu* pMenu) : pMenu(pMenu) {}
        virtual void onClick(pk::InputMouseButtonName button);
    };

    friend class SetTemperatureEvent;
    friend class TemperatureSelection;
    friend class OpenTemperatureSelectionEvent;
    friend class OnClickApply;

    TemperatureSelection _temperatureSelection;
    TypeSelection _typeSelection;


    pk::ui::InputField _selectedRadiusInputField;
    pk::ui::InputField _selectedElevationInputField;
    // These mean what properties we have currently selected to apply
    // NOT what currently is in the selected tile!
    gamecommon::TileStateTemperature _selectedTemperature;
    gamecommon::TileStateTerrType _selectedType;

    entityID_t _selectedTemperatureTxtEntity = 0;
    entityID_t _selectedTypeTxtEntity = 0;

    uint64_t _selectedTileData = 0;
    int32_t _selectedTileX = 0;
    int32_t _selectedTileY = 0;

public:
    void init(pk::Scene* pScene, pk::Font* pFont);

    virtual void open();
    virtual void close();

    void setSelectedTile(uint64_t tileData, int32_t x, int32_t y);
    void setSelectedTemperature(gamecommon::TileStateTemperature temperature);
    void setSelectedType(gamecommon::TileStateTerrType type);

    inline pk::ui::InputField& getSelectedRadiusInputField() { return _selectedRadiusInputField; }
    inline pk::ui::InputField& getSelectedElevationInputField() { return _selectedElevationInputField; }

    inline int32_t getSelectedTileX() const { return _selectedTileX; }
    inline int32_t getSelectedTileY() const { return _selectedTileY; }
};
