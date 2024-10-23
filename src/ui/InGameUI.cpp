#include "InGameUI.h"
#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/Tile.h"
#include "Object.h"
#include "world/Objects.h"


using namespace pk;
using namespace world;


// TODO:
// * Some func to add and store status and attribute strings/values more clearly
// * Display "really" selected object's info
void InGameUI::create(pk::Scene* pScene, pk::Font* pFont)
{
    _pScene = pScene;

    const vec2 settingsPanelScale(212, 25);
    const vec2 settingsPanelSlotScale(100, 24);
    _settingsPanel.createDefault(
        pScene,
        pFont,
        HorizontalConstraintType::PIXEL_RIGHT, 0.0f,
        VerticalConstraintType::PIXEL_TOP, 0.0f,
        settingsPanelScale,
        settingsPanelSlotScale,
        Panel::LayoutFillType::HORIZONTAL
    );
    _settingsPanel.addDefaultButton(
        "Logout",
        nullptr,
        100
    );

    _settingsPanel.addDefaultButton(
        "Settings",
        nullptr,
        100
    );

    const vec2 selectedPanelScale(600, 140);
    const vec2 selectedPanelSlotScale(200, 30);
    _selectedPanel.createDefault(
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
    _selectedPanel.addImage(
        HorizontalConstraintType::PIXEL_LEFT, 0.0f,
        VerticalConstraintType::PIXEL_BOTTOM, selectedPanelScale.y,
        selectedPanelScale.x, clh,
        nullptr, // texture
        Panel::get_base_ui_color(2).toVec3(),
        { 0, 0, 1, 1 } // texture cropping
    );


    // Atm just testing some random texture as portrait to see how it looks..
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
    const int portraitTextureRows = 16;
    const float portraitCroppingScale = 1.0f / (float)portraitTextureRows;
    entityID_t selectedPortraitEntity = _selectedPanel.addImage(
        HorizontalConstraintType::PIXEL_LEFT, 32.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f,
        portraitWidth, portraitHeight,
        pPortraiTexture, // texture
        { 1, 1, 1 },
        { 0, 15, portraitCroppingScale, portraitCroppingScale } // texture cropping
    );

    _objectNameEntity = _selectedPanel.addText(
        "Rifleman",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight
    ).first;
    const float textHeight = 20.0f; // can be found from BaseScene TODO: make that variable!
    const float propertiesTxtPaddingY = 4.0f;
    const float infoColumnWidth = 140.0f;

    // Draw line under "Status" and "Attributes"
    _selectedPanel.addImage(
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - textHeight - 2,
        infoColumnWidth * 3, 1,
        nullptr, // texture
        Panel::get_base_ui_color(3).toVec3(),
        { 0, 0, 1, 1 } // texture cropping
    );

    vec2 infoTxtPos(32.0f + portraitWidth + 8.0f, 42.0f + portraitHeight);

    _statusInfoEntities = addInfoColumn(
        0,
        infoColumnWidth,
        infoTxtPos,
        propertiesTxtPaddingY,
        textHeight,
        "Status",
        {
            "Health: 10",
            "Stamina: 2",
            "Mental: 8"
        }
    );

    _attributeInfoEntities = addInfoColumn(
        1,
        infoColumnWidth,
        infoTxtPos,
        propertiesTxtPaddingY,
        textHeight,
        "Attributes",
        {
            "Speed: 4",
            "Strength: 5",
            "Accuracy: 2",
            "Armor: 3"
        }
    );

    // Draw line before "terrain data"
    _selectedPanel.addImage(
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
            "Effect: None",
            "Position: 0, 0"
        }
    );
}

void InGameUI::setSelectedInfo(uint64_t tile, int tileX, int tileY)
{
    GC_ubyte object = gamecommon::get_tile_thingid(tile);
    GC_ubyte tileElevation = gamecommon::get_tile_terrelevation(tile);
    gamecommon::ObjectInfo* pObjectInfo = objects::ObjectInfoLib::get(object);
    std::string objNameStr(pObjectInfo->name, OBJECT_DATA_STRLEN_NAME);

    // Set obj name txt
    TextRenderable* pObjNameTxt = (TextRenderable*)_pScene->getComponent(
        _objectNameEntity,
        ComponentType::PK_RENDERABLE_TEXT
    );
    pObjNameTxt->accessStr() = objNameStr;

    // Tile info
    TextRenderable* pTileInfoElevationTxt = (TextRenderable*)_pScene->getComponent(
        _tileInfoEntities[TileInfoSlotIndex::elevation],
        ComponentType::PK_RENDERABLE_TEXT
    );
    pTileInfoElevationTxt->accessStr() = "Elevation: " + std::to_string(tileElevation);

    TextRenderable* pTileInfoPosTxt = (TextRenderable*)_pScene->getComponent(
        _tileInfoEntities[TileInfoSlotIndex::position],
        ComponentType::PK_RENDERABLE_TEXT
    );
    pTileInfoPosTxt->accessStr() = "Location: " + std::to_string(tileX) + ", " + std::to_string(tileY);
}

// Returns created info txt entities (doesn't include the title)
std::vector<entityID_t> InGameUI::addInfoColumn(
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
    _selectedPanel.addText(
        title,
        HorizontalConstraintType::PIXEL_LEFT, pos.x + 8.0f + columnWidth * columnIndex,
        VerticalConstraintType::PIXEL_BOTTOM, pos.y - txtPaddingY - txtHeight
    );

    for (int i = 0; i < infos.size(); ++i)
    {
        infoTxtEntities[i] = _selectedPanel.addText(
            infos[i],
            HorizontalConstraintType::PIXEL_LEFT, pos.x + 8.0f + columnWidth * columnIndex,
            VerticalConstraintType::PIXEL_BOTTOM, pos.y - txtPaddingY - (txtHeight * (i + 2) + txtPaddingY)
        ).first;
    }
    return infoTxtEntities;
}
