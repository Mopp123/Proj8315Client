#include "SelectedPanel.h"

#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/Tile.h"
#include "Object.h"
#include "world/Objects.h"


using namespace pk;
using namespace world;
using namespace gamecommon;


static std::string temperature_value_to_string(TileStateTemperature value)
{
    switch (value)
    {
        case TileStateTemperature::TILE_STATE_mild : return "Mild";
        case TileStateTemperature::TILE_STATE_warm : return "Warm";
        case TileStateTemperature::TILE_STATE_hot : return "Hot";
        case TileStateTemperature::TILE_STATE_burning : return "Burning";
        case TileStateTemperature::TILE_STATE_chilly : return "Chilly";
        case TileStateTemperature::TILE_STATE_cold : return "Cold";
        case TileStateTemperature::TILE_STATE_freezing : return "Freezing";
        default : return "Invalid temperature value";
    }
}


void SelectedPanel::init(pk::Scene* pScene, pk::Font* pFont)
{
    const vec2 selectedPanelScale(630, 160);
    const vec2 selectedPanelSlotScale(200, 30);
    createDefault(
        pScene,
        pFont,
        HorizontalConstraintType::PIXEL_LEFT, 0.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 0.0f,
        selectedPanelScale,
        selectedPanelSlotScale,
        Panel::LayoutFillType::HORIZONTAL,
        0
    );
    // Draw little cosmetic line on "selected panel"
    const float clh = 20;
    addImage(
        HorizontalConstraintType::PIXEL_LEFT, 0.0f,
        VerticalConstraintType::PIXEL_BOTTOM, selectedPanelScale.y - clh,
        selectedPanelScale.x, clh,
        nullptr, // texture
        Panel::get_base_ui_color(2).toVec3(),
        { 0, 0, 1, 1 } // texture cropping
    );

    ResourceManager& resManager = Application::get()->getResourceManager();
    ImageData* pPortraitImage = resManager.loadImage(
        "assets/textures/UnitPortraits.png",
        true
    );
    TextureSampler portraitTexSampler(
        TextureSamplerFilterMode::PK_SAMPLER_FILTER_MODE_NEAR,
        TextureSamplerAddressMode::PK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    );
    Texture* pPortraiTexture = resManager.createTexture(
        pPortraitImage->getResourceID(),
        portraitTexSampler
    );
    const float portraitWidth = 100;
    const float portraitHeight = 100;
    const float portraitCroppingScale = 1.0f / (float)_portraitTextureRows;
    _selectedPortraitEntity = addImage(
        HorizontalConstraintType::PIXEL_LEFT, 32.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f,
        portraitWidth, portraitHeight,
        pPortraiTexture, // texture
        { 1, 1, 1 },
        { 0, 15, portraitCroppingScale, portraitCroppingScale } // texture cropping
    );

    _objectNameEntity = addText(
        "",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight
    ).first;
    const float textHeight = 20.0f; // can be found from BaseScene TODO: make that variable!
    const float propertiesTxtPaddingY = 4.0f;
    const float infoColumnWidth = 140.0f;

    // Draw line under "Status" and "Attributes"
    addImage(
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - textHeight - 2,
        infoColumnWidth * 3, 1,
        nullptr, // texture
        Panel::get_base_ui_color(3).toVec3(),
        { 0, 0, 1, 1 } // texture cropping
    );

    vec2 infoTxtPos(32.0f + portraitWidth + 8.0f, 42.0f + portraitHeight);

    // TODO: Object stat system
    _statusInfoEntities = addInfoColumn(
        0,
        infoColumnWidth,
        infoTxtPos,
        propertiesTxtPaddingY,
        textHeight,
        "Status",
        {
        /*
            "Health: 10",
            "Stamina: 2",
            "Mental: 8"
        */
        }
    );

    // TODO: Object attribute system
    _attributeInfoEntities = addInfoColumn(
        1,
        infoColumnWidth,
        infoTxtPos,
        propertiesTxtPaddingY,
        textHeight,
        "Attributes",
        {
        /*
            "Speed: 4",
            "Strength: 5",
            "Accuracy: 2",
            "Armor: 3"
        */
        }
    );

    // Draw line before "terrain data"
    addImage(
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f + infoColumnWidth * 2 + 2.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - 105,
        1, 105,
        nullptr, // texture
        Panel::get_base_ui_color(3).toVec3(),
        { 0, 0, 1, 1 } // texture cropping
    );

    _tileInfoEntities = addInfoColumn(
        2,
        infoColumnWidth,
        infoTxtPos,
        propertiesTxtPaddingY,
        textHeight,
        "Environment",
        {
            "Type: Barren",
            "Elevation: 1",
            "Temperature:",
            "Effect: None",
            "Position: 0, 0"
        }
    );
}

void SelectedPanel::setSelectedInfo(uint64_t tile, int tileX, int tileY)
{
    GC_ubyte object = gamecommon::get_tile_thingid(tile);
    GC_ubyte tileElevation = gamecommon::get_tile_terrelevation(tile);
    GC_ubyte tileTemperature = gamecommon::get_tile_temperature(tile);
    gamecommon::ObjectInfo* pObjectInfo = objects::ObjectInfoLib::get(object);
    std::string objNameStr(pObjectInfo->name, OBJECT_DATA_STRLEN_NAME);

    // Set obj name txt
    TextRenderable* pObjNameTxt = (TextRenderable*)_pScene->getComponent(
        _objectNameEntity,
        ComponentType::PK_RENDERABLE_TEXT
    );
    pObjNameTxt->accessStr() = objNameStr;

    // Set portrait
    // NOTE: Atm assuming object ordering matches the "portraits texture atlas"
    GUIRenderable* pPortraitImg = (GUIRenderable*)_pScene->getComponent(
        _selectedPortraitEntity,
        ComponentType::PK_RENDERABLE_GUI
    );
    // Break down the objectID/index into x and y components in the "portraits img grid"
    int portraitX = (int)object % _portraitTextureRows;
    // *also make first row be the top row...
    int portraitY = (_portraitTextureRows - 1) - ((int)object / _portraitTextureRows);
    if (portraitX >= _portraitTextureRows || portraitY >= _portraitTextureRows)
    {
        Debug::log(
            "@SelectedPanel::setSelectedInfo "
            "Not enough portraint slots for object: " + objNameStr + " (" + std::to_string(object) + ") "
            "Requested slot coords were: " + std::to_string(portraitX) + ", " + std::to_string(portraitY) + " "
            "Current available slots: " + std::to_string(_portraitTextureRows),
            Debug::MessageType::PK_ERROR
        );
    }
    pPortraitImg->textureCropping.x = portraitX;
    pPortraitImg->textureCropping.y = portraitY;


    // Tile info
    TextRenderable* pTileInfoElevationTxt = (TextRenderable*)_pScene->getComponent(
        _tileInfoEntities[TileInfoSlotIndex::elevation],
        ComponentType::PK_RENDERABLE_TEXT
    );
    pTileInfoElevationTxt->accessStr() = "Elevation: " + std::to_string(tileElevation);

    TextRenderable* pTileInfoTemperatureTxt = (TextRenderable*)_pScene->getComponent(
        _tileInfoEntities[TileInfoSlotIndex::temperature],
        ComponentType::PK_RENDERABLE_TEXT
    );
    pTileInfoTemperatureTxt->accessStr() = "Temperature: " + temperature_value_to_string((TileStateTemperature)tileTemperature);

    TextRenderable* pTileInfoPosTxt = (TextRenderable*)_pScene->getComponent(
        _tileInfoEntities[TileInfoSlotIndex::position],
        ComponentType::PK_RENDERABLE_TEXT
    );
    pTileInfoPosTxt->accessStr() = "Location: " + std::to_string(tileX) + ", " + std::to_string(tileY);
}

// Returns created info txt entities (doesn't include the title)
std::vector<entityID_t> SelectedPanel::addInfoColumn(
    int columnIndex,
    float columnWidth,
    const pk::vec2& pos,
    float txtPaddingY,
    float txtHeight,
    const std::string& title,
    const std::vector<std::string>& infos
)
{
    std::vector<entityID_t> infoTxtEntities(infos.size());
    addText(
        title,
        HorizontalConstraintType::PIXEL_LEFT, pos.x + 8.0f + columnWidth * columnIndex,
        VerticalConstraintType::PIXEL_BOTTOM, pos.y - txtPaddingY - txtHeight
    );

    for (int i = 0; i < infos.size(); ++i)
    {
        infoTxtEntities[i] = addText(
            infos[i],
            HorizontalConstraintType::PIXEL_LEFT, pos.x + 8.0f + columnWidth * columnIndex,
            VerticalConstraintType::PIXEL_BOTTOM, pos.y - txtPaddingY - (txtHeight * (i + 2) + txtPaddingY)
        ).first;
    }
    return infoTxtEntities;
}
