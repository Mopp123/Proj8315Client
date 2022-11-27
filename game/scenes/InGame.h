#pragma once

#include "../../PortablePesukarhu/ppk.h"

#include "../world/World.h"

#include <vector>


class InGame : public pk::Scene
{
private:

    pk::ui::Text* _pText_debug_delta = nullptr;

    world::VisualWorld* _visualWorld = nullptr;

    pk::Sprite3DRenderable* _testSprite = nullptr;

    pk::RTSCamController* _pCamController = nullptr;
    pk::mat4* _pCamTransform = nullptr;

    pk::web::WebTexture* _terrainTexture0 = nullptr;
    pk::web::WebTexture* _terrainTexture1 = nullptr;
    pk::web::WebTexture* _terrainTexture2 = nullptr;
    pk::web::WebTexture* _terrainTexture3 = nullptr;
    pk::web::WebTexture* _terrainTexture4 = nullptr;

    pk::ui::InputField* _inputField_position = nullptr;

public:

    InGame();
    ~InGame();

    virtual void init();
    virtual void update();

};
