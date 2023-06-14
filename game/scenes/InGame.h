#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "BaseScene.h"
#include "../net/Client.h"
#include "../world/World.h"
#include <vector>


class InGame : public BaseScene
{
private:
    // On message events
    class OnMessageGetAllFactions : public net::OnMessageEvent
    {
    public:
        world::World& worldRef;

        OnMessageGetAllFactions(world::World& worldRef) :
            worldRef(worldRef)
        {}

        virtual void onMessage(const PK_byte* data, size_t dataSize);
    };

    class OnMessageGetChangedFactions : public net::OnMessageEvent
    {
    public:
        world::World& worldRef;

        OnMessageGetChangedFactions(world::World& worldRef) :
            worldRef(worldRef)
        {}

        virtual void onMessage(const PK_byte* data, size_t dataSize);
    };

    world::World* _world = nullptr;

    pk::Sprite3DRenderable* _testSprite = nullptr;

    pk::RTSCamController* _pCamController = nullptr;
    pk::mat4* _pCamTransform = nullptr;

    pk::web::WebTexture* _terrainTexture0 = nullptr;
    pk::web::WebTexture* _terrainTexture1 = nullptr;
    pk::web::WebTexture* _terrainTexture2 = nullptr;
    pk::web::WebTexture* _terrainTexture3 = nullptr;
    pk::web::WebTexture* _terrainTexture4 = nullptr;

public:
    InGame();
    ~InGame();

    virtual void init();
    virtual void update();

};
