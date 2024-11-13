#include "TileOptionsMenu.h"
#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/Tile.h"
#include "Object.h"
#include "world/Objects.h"


using namespace pk;
using namespace world;
using namespace gamecommon;


void TileOptionsMenu::init(pk::Scene* pScene, pk::Font* pFont)
{
    const vec2 panelScale(122, 160);
    const vec2 slotScale(130, 20);
    createDefault(
        pScene,
        pFont,
        HorizontalConstraintType::PIXEL_LEFT, 0.0f,
        VerticalConstraintType::PIXEL_BOTTOM, 60.0f,
        panelScale,
        slotScale,
        Panel::LayoutFillType::VERTICAL,
        0
    );

    for (int i = 0; i < _maxSlotItems; ++i)
    {
        _slotEntities.push_back(
            addDefaultButton("Item" + std::to_string(i), nullptr, slotScale.x)
        );
    }
}

void TileOptionsMenu::show(uint64_t tileData, float x, float y)
{
    Debug::log("___TEST___ display options at: " + std::to_string(x) + ", " + std::to_string(y));

    Transform* pTransform = (Transform*)_pScene->getComponent(
        _entity,
        ComponentType::PK_TRANSFORM
    );

    ConstraintData* pConstraintData = (ConstraintData*)_pScene->getComponent(
        _entity,
        ComponentType::PK_UI_CONSTRAINT
    );
    pConstraintData->horizontalValue = x;
    pConstraintData->verticalValue = y;

    /*
    for (entityID_t e : _slotEntities)
    {
        Debug::log("___TEST___set slot constraint data");
        ConstraintData* pSlotConstraintData = (ConstraintData*)_pScene->getComponent(
            e,
            ComponentType::PK_UI_CONSTRAINT
        );
        pSlotConstraintData->horizontalValue = x;
        pSlotConstraintData->verticalValue = y;
    }*/
}

void TileOptionsMenu::hide()
{}
