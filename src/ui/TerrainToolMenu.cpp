#include "TerrainToolMenu.h"
#include "../../Proj8315Common/src/Tile.h"
#include "../../Proj8315Common/src/messages/AdminMessages.h"
#include "Object.h"
#include "messages/Message.h"
#include "world/Objects.h"
#include "net/Client.h"


using namespace pk;
using namespace pk::ui;
using namespace world;
using namespace world::objects;
using namespace gamecommon;
using namespace net;


void TerrainToolMenu::SetValueEvent::onClick(pk::InputMouseButtonName button)
{
    if (type == EventType::SET_TEMPERATURE)
        pToolMenu->setSelectedTemperature((TileStateTemperature)value);
    else if (type == EventType::SET_TYPE)
        pToolMenu->setSelectedType((TileStateTerrType)value);
    pSelectionMenu->close();
}


void TerrainToolMenu::TemperatureSelection::init(pk::Scene* pScene, pk::Font* pFont)
{
    vec2 scale(182, 230);
    initBase(
        pScene,
        pFont,
        "Select Temperature",
        {
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            -scale.x * 0.5f,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL,
            scale.y * 0.5f
        },
        scale,
        LayoutFillType::VERTICAL
    );

    const float buttonWidth = 100.0f;
    addDefaultButton(
        "Freezing",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TEMPERATURE,
            TileStateTemperature::TILE_STATE_freezing
        ),
        buttonWidth
    );
    addDefaultButton(
        "Cold",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TEMPERATURE,
            TileStateTemperature::TILE_STATE_cold
        ),
        buttonWidth
    );
    addDefaultButton(
        "Chilly",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TEMPERATURE,
            TileStateTemperature::TILE_STATE_chilly
        ),
        buttonWidth
    );
    addDefaultButton(
        "Mild",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TEMPERATURE,
            TileStateTemperature::TILE_STATE_mild
        ),
        buttonWidth
    );
    addDefaultButton(
        "Warm",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TEMPERATURE,
            TileStateTemperature::TILE_STATE_warm
        ),
        buttonWidth
    );
    addDefaultButton(
        "Hot",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TEMPERATURE,
            TileStateTemperature::TILE_STATE_hot
        ),
        buttonWidth
    );
    addDefaultButton(
        "Burning",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TEMPERATURE,
            TileStateTemperature::TILE_STATE_burning
        ),
        buttonWidth
    );

    setLayer(2);

    close();
}

void TerrainToolMenu::TemperatureSelection::open()
{
    setComponentsActive(true);
}

void TerrainToolMenu::TemperatureSelection::close()
{
    setComponentsActive(false);
}

void TerrainToolMenu::TypeSelection::init(pk::Scene* pScene, pk::Font* pFont)
{
    vec2 scale(182, 230);
    initBase(
        pScene,
        pFont,
        "Select Temperature",
        {
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            -scale.x * 0.5f,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL,
            scale.y * 0.5f,
        },
        scale,
        LayoutFillType::VERTICAL
    );

    const float buttonWidth = 100.0f;
    addDefaultButton(
        "Water",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TYPE,
            TileStateTerrType::TILE_STATE_terrTypeWater
        ),
        buttonWidth
    );
    addDefaultButton(
        "Dirt",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TYPE,
            TileStateTerrType::TILE_STATE_terrTypeDirt
        ),
        buttonWidth
    );
    addDefaultButton(
        "Rock",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TYPE,
            TileStateTerrType::TILE_STATE_terrTypeRock
        ),
        buttonWidth
    );
    addDefaultButton(
        "Fertile Soil",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TYPE,
            TileStateTerrType::TILE_STATE_terrTypeFertile
        ),
        buttonWidth
    );
    addDefaultButton(
        "Dunes",
        new SetValueEvent(
            pMenu,
            this,
            SetValueEvent::EventType::SET_TYPE,
            TileStateTerrType::TILE_STATE_terrTypeDunes
        ),
        buttonWidth
    );

    setLayer(2);

    close();
}

void TerrainToolMenu::TypeSelection::open()
{
    setComponentsActive(true);
}

void TerrainToolMenu::TypeSelection::close()
{
    setComponentsActive(false);
}


void TerrainToolMenu::OpenSelectionEvent::onClick(pk::InputMouseButtonName button)
{
    if (selection == SelectionType::TEMPERATURE)
        pMenu->_temperatureSelection.open();
    else if (selection == SelectionType::TYPE)
        pMenu->_typeSelection.open();
}


void TerrainToolMenu::OnClickApply::onClick(pk::InputMouseButtonName button)
{
    Client* pClient = Client::get_instance();
    if (!pClient)
    {
        Debug::log(
            "@TerrainToolMenu::OnClickApply::onClick "
            "Client was nullptr!",
            Debug::MessageType::PK_ERROR
        );
        return;
    }

    const std::string& radiusTxt = pMenu->getSelectedRadius();
    const std::string& elevationTxt = pMenu->getSelectedElevation();
    GC_ubyte radius = 1;
    GC_ubyte elevation = 0;
    // TODO: Maybe some error handling?
    if (!radiusTxt.empty())
        radius = (GC_ubyte)std::stoi(radiusTxt);
    if (!elevationTxt.empty())
        elevation = (GC_ubyte)std::stoi(elevationTxt);

    pClient->send(
        (int32_t)MESSAGE_TYPE__TerrainModRequest,
        {
            {
                (GC_byte*)&radius, 1, 1
            },
            {
                (GC_byte*)&pMenu->_selectedTileX,
                sizeof(int32_t),
                sizeof(int32_t)
            },
            {
                (GC_byte*)&pMenu->_selectedTileY,
                sizeof(int32_t),
                sizeof(int32_t)
            },
            {
                (GC_byte*)&elevation, 1, 1
            },
            {
                (GC_byte*)&pMenu->_selectedTemperature, 1, 1
            },
            {
                (GC_byte*)&pMenu->_selectedType, 1, 1
            },
        }
    );

    pMenu->close();
}


void TerrainToolMenu::init(pk::Scene* pScene, pk::Font* pFont)
{
    vec2 scale(400, 260);
    initBase(
        pScene,
        pFont,
        "Admin Terrain Tool",
        {
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            -scale.x * 0.5f,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL,
            scale.y * 0.5f
        },
        scale,
        LayoutFillType::VERTICAL
    );

    _temperatureSelection.pMenu = this;
    _temperatureSelection.init(pScene, pFont);

    _typeSelection.pMenu = this;
    _typeSelection.init(pScene, pFont);

    addDefaultText("NOTE: Elevation needs to be between 0 and 31");

    float numberInputFieldWidth = 50.0f;
    _selectedRadiusInputField = addDefaultInputField(
        "Apply radius",
        numberInputFieldWidth,
        nullptr
    );
    setRadiusInputFieldContent("1");

    _selectedElevationInputField = addDefaultInputField(
        "Elevation",
        numberInputFieldWidth,
        nullptr
    );
    setElevationInputFieldContent("15");

    addDefaultButton(
        "Temperature",
        new OpenSelectionEvent(this, OpenSelectionEvent::SelectionType::TEMPERATURE),
        110.0f
    );
    _selectedTemperatureTxtEntity = addDefaultText("");

    addDefaultButton(
        "Type",
        new OpenSelectionEvent(this, OpenSelectionEvent::SelectionType::TYPE),
        110.0f
    );
    _selectedTypeTxtEntity = addDefaultText("");

    addDefaultButton(
        "Apply",
        new OnClickApply(this),
        70.0f
    );

    setSelectedTemperature(TileStateTemperature::TILE_STATE_mild);
    setSelectedType(TileStateTerrType::TILE_STATE_terrTypeDirt);

    close();
    setLayer(1);
}

void TerrainToolMenu::open()
{
    setComponentsActive(true);
}

void TerrainToolMenu::close()
{
    setComponentsActive(false);
}

void TerrainToolMenu::setSelectedTile(uint64_t tileData, int32_t x, int32_t y)
{
    _selectedTileData = tileData;
    _selectedTileX = x;
    _selectedTileY = y;
    setElevationInputFieldContent(std::to_string(get_tile_terrelevation(_selectedTileData)));
}

void TerrainToolMenu::setSelectedTemperature(gamecommon::TileStateTemperature temperature)
{
    _selectedTemperature = temperature;
    TextRenderable* pRenderable = (TextRenderable*)_pScene->getComponent(
        _selectedTemperatureTxtEntity,
        ComponentType::PK_RENDERABLE_TEXT
    );
    pRenderable->accessStr() = "    Selected: " + temperature_value_to_string(_selectedTemperature);
}

void TerrainToolMenu::setSelectedType(gamecommon::TileStateTerrType type)
{
    _selectedType = type;
    TextRenderable* pRenderable = (TextRenderable*)_pScene->getComponent(
        _selectedTypeTxtEntity,
        ComponentType::PK_RENDERABLE_TEXT
    );
    pRenderable->accessStr() = "    Selected: " + terrain_type_value_to_string(_selectedType, _selectedTemperature);
}

const std::string& TerrainToolMenu::getSelectedRadius() const
{
    const UIElemState* pState = (const UIElemState*)_pScene->getComponent(
        _selectedRadiusInputField.rootEntity,
        ComponentType::PK_UIELEM_STATE
    );
    return pState->content;
}

const std::string& TerrainToolMenu::getSelectedElevation() const
{
    const UIElemState* pState = (const UIElemState*)_pScene->getComponent(
        _selectedElevationInputField.rootEntity,
        ComponentType::PK_UIELEM_STATE
    );
    return pState->content;
}

void TerrainToolMenu::setRadiusInputFieldContent(const std::string& str)
{
    set_input_field_content(
        str,
        _selectedRadiusInputField.rootEntity,
        _selectedRadiusInputField.contentEntity
    );
}

void TerrainToolMenu::setElevationInputFieldContent(const std::string& str)
{
    set_input_field_content(
        str,
        _selectedElevationInputField.rootEntity,
        _selectedElevationInputField.contentEntity
    );
}
