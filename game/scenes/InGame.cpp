#include "InGame.h"

#include "../net/Client.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "../../Proj8315Common/src/messages/WorldMessages.h"
#include "../../Proj8315Common/src/Faction.h"

#include "../world/Tile.h"

#include <cmath>
#include <iostream>
#include <chrono>


using namespace pk;
using namespace pk::web;
using namespace ui;
using namespace net;
using namespace gamecommon;


void InGame::OnMessageFactionList::onMessage(const PK_byte* data, size_t dataSize)
{
    FactionListResponse response(data, dataSize);
    for (const Faction& faction : response.getFactions())
        worldRef.addFaction(faction);
    pInGameScene->factionListInitialized = true;
}


void InGame::OnMessageUpdatedFactionList::onMessage(const PK_byte* data, size_t dataSize)
{
    // Prevent conflict if receiving updated factions before "initial" factions list
    if (!pInGameScene->factionListInitialized)
        return;
    // Debug::log(
    //     "___TEST___InGame::OnMessageGetChangedFactions::onMessage NOT YET IMPLEMENTED!",
    //     Debug::MessageType::PK_WARNING
    // );
    UpdatedFactionsMsg msg(data, dataSize);
    for (const Faction& faction : msg.getFactions())
    {
        worldRef.updateFactionList(faction.getName(), faction);
    }
}


InGame::InGame()
{}

InGame::~InGame()
{
    delete _pCamController;

    delete _world;
    // NOTE: system deleting issue! UPDATE: this is very old comment, may not apply anymore -> INVESTIGATE FURTHER!
    //delete _pText_debug_delta;

    delete _terrainTexture0;
    delete _terrainTexture1;
    delete _terrainTexture2;
    delete _terrainTexture3;
    delete _terrainTexture4;
}

static std::string s_TEST_worldstate;

void InGame::init()
{
    ((BaseScene*)this)->initBase();
     _debugPanel.setActive(true);

    // create dir light
    uint32_t lightEntity = createEntity();
    vec3 lightDir(0.0f, -0.5f, -0.5f);
    lightDir.normalize();
    addComponent(lightEntity, new DirectionalLight({ 1,1,1 }, lightDir));

    std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::high_resolution_clock::now();

    // Load channel textures for terrain's tiles
    TextureSampler textureSampler =
    {
        TextureSamplerFilterMode::PK_SAMPLER_FILTER_MODE_LINEAR,
        TextureSamplerAddressMode::PK_SAMPLER_ADDRESS_MODE_REPEAT,
        2
    };
    _terrainTexture0 = new WebTexture("assets/deadland.png", textureSampler);
    _terrainTexture1 = new WebTexture("assets/water.png",	 textureSampler);
    _terrainTexture2 = new WebTexture("assets/snow.png", 	 textureSampler);
    _terrainTexture3 = new WebTexture("assets/rock.png", 	 textureSampler);
    _terrainTexture4 = new WebTexture("assets/grass.png",	 textureSampler);

    TerrainTileRenderable::s_channelTexture0 = (Texture*)_terrainTexture0;
    TerrainTileRenderable::s_channelTexture1 = (Texture*)_terrainTexture1;
    TerrainTileRenderable::s_channelTexture2 = (Texture*)_terrainTexture2;
    TerrainTileRenderable::s_channelTexture3 = (Texture*)_terrainTexture3;
    TerrainTileRenderable::s_channelTexture4 = (Texture*)_terrainTexture4;

    // Load objects sprite sheet texture
    TextureSampler spriteTextureSampler =
    {
        TextureSamplerFilterMode::PK_SAMPLER_FILTER_MODE_LINEAR,
        TextureSamplerAddressMode::PK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        2
    };

    Transform* camTransformComponent = (Transform*)((Scene*)this)->getComponent(
        activeCamera->getEntity(), ComponentType::PK_TRANSFORM
    );
    _world = new world::World((Scene&)*this, camTransformComponent, 15);

    float delta = (std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime)).count();

    Debug::log("building took: " + std::to_string(delta));

    _pCamTransform = &(camTransformComponent)->accessTransformationMatrix();
    _pCamTransform->setIdentity();
    mat4& camTransform = *_pCamTransform;

    _pCamController = new RTSCamController(*activeCamera, (Scene*)this);
    _pCamController->setPivotPoint({125.0f, 0, 125.0f});

    // TESTING 3D sprites..
    Texture* testSpriteTexture = (Texture*)(new WebTexture("assets/environment.png", textureSampler, 8));

    uint32_t spriteEntity = createEntity();
    _testSprite = new Sprite3DRenderable({ 0,0,0 }, { 2,2 }, testSpriteTexture);
    addComponent(spriteEntity, _testSprite);

    Client* client = Client::get_instance();
    // TODO: Fix below!
    client->addOnMessageEvent(MESSAGE_TYPE__FactionListResponse, new OnMessageFactionList(this, *_world));
    client->addOnMessageEvent(MESSAGE_TYPE__UpdatedFactions, new OnMessageUpdatedFactionList(this, *_world));

    // Fetch all existing game factions
     Client::get_instance()->send(MESSAGE_TYPE__FactionListRequest, {});
}

void InGame::update()
{
    updateDebugPanel();

    vec3 camPivotPoint = _pCamController->getPivotPoint();

    // attempt to glue cam's height to terrain's height
    camPivotPoint.y = _world->getTileVisualHeightAt(camPivotPoint.x, camPivotPoint.z);
    _pCamController->setPivotPoint(camPivotPoint);

    _world->update(camPivotPoint.x, camPivotPoint.z);

    // debug mouse picking testing..
    mat4 viewMatrix = *_pCamTransform;
    viewMatrix.inverse();

    _testSprite->position = _world->getMousePickCoords(activeCamera->getProjMat3D(), viewMatrix);
}
