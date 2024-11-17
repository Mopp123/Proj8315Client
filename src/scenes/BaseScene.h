#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "ecs/components/ui/ConstraintData.h"


class BaseScene : public pk::Scene
{
protected:
    pk::Font* _pDefaultFont = nullptr;
    pk::Font* _pSmallFont = nullptr;
    pk::TextRenderable* _pInfoTxt = nullptr;

    entityID_t _infoTextEntity = NULL_ENTITY_ID;

public:
    BaseScene();
    virtual ~BaseScene();
    void initBase();

    void setInfoText(
        const std::string& txt,
        pk::vec3 color = pk::vec3(1.0f, 1.0f, 1.0f),
        float horizontalVal = 0,
        float verticalVal = 0,
        pk::HorizontalConstraintType horizontalConstraint = pk::HorizontalConstraintType::PIXEL_LEFT,
        pk::VerticalConstraintType verticalConstraint = pk::VerticalConstraintType::PIXEL_TOP
    );
};
