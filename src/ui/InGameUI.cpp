#include "InGameUI.h"


using namespace pk;


// TODO:
// * Some func to add and store status and attribute strings/values more clearly
// * Display "really" selected object's info

void InGameUI::create(pk::Scene* pScene, pk::Font* pFont)
{
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

    const vec2 selectedPanelScale(420, 140);
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

    _selectedPanel.addText(
        "Rifleman",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight
    );
    const float textHeight = 20.0f; // can be found from BaseScene TODO: make that variable!
    const float propertiesTxtPaddingY = 4.0f;
    _selectedPanel.addText(
        "Status",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY -textHeight
    );
    // Draw line under "Status"
    _selectedPanel.addImage(
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - textHeight - 2,
        240, 1,
        nullptr, // texture
        Panel::get_base_ui_color(3).toVec3(),
        { 0, 0, 1, 1 } // texture cropping
    );

    _selectedPanel.addText(
        "Health: 12",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - (textHeight * 2 + propertiesTxtPaddingY)
    );
    _selectedPanel.addText(
        "Stamina: 5",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - (textHeight * 3 + propertiesTxtPaddingY)
    );
    _selectedPanel.addText(
        "Mental: 8",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - (textHeight * 4 + propertiesTxtPaddingY)
    );

    _selectedPanel.addText(
        "Attributes",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f + 120.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - textHeight
    );
    _selectedPanel.addText(
        "Speed: 4",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f + 120.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - (textHeight * 2 + propertiesTxtPaddingY)
    );
    _selectedPanel.addText(
        "Strength: 2",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f + 120.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - (textHeight * 3 + propertiesTxtPaddingY)
    );
    _selectedPanel.addText(
        "Accuracy: 10",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f + 120.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - (textHeight * 4 + propertiesTxtPaddingY)
    );
    _selectedPanel.addText(
        "Armor: 5",
        HorizontalConstraintType::PIXEL_LEFT, 32.0f + portraitWidth + 8.0f + 120.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 42.0f + portraitHeight - propertiesTxtPaddingY - (textHeight * 5 + propertiesTxtPaddingY)
    );
}
