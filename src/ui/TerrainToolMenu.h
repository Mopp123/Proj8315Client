#pragma once

#include "Tile.h"
#include "TopBarPanel.h"
#include "../../Proj8315Common/src/Object.h"
#include "ecs/factories/ui/UIFactories.h"
#include <vector>


class TerrainToolMenu : public TopBarPanel
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

    class SetValueEvent : public pk::ui::OnClickEvent
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

    class OpenSelectionEvent : public pk::ui::OnClickEvent
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

    class OnClickApply : public pk::ui::OnClickEvent
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


    pk::ui::UIFactoryInputField _selectedRadiusInputField;
    pk::ui::UIFactoryInputField _selectedElevationInputField;
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

    const std::string& getSelectedRadius() const;
    const std::string& getSelectedElevation() const;

    inline int32_t getSelectedTileX() const { return _selectedTileX; }
    inline int32_t getSelectedTileY() const { return _selectedTileY; }

private:
    void setRadiusInputFieldContent(const std::string& str);
    void setElevationInputFieldContent(const std::string& str);
};