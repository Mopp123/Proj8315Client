#include <algorithm>
#include <chrono>
#include <cstring>
#include <string>

#include "Objects.h"
#include "World.h"
#include "Tile.h"
#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "../../Proj8315Common/src/messages/WorldMessages.h"

#include <chrono>

using namespace pk;
using namespace pk::web;
using namespace gamecommon;

using namespace net;


namespace world
{
    using namespace objects;


    static float s_idleAnimSpeed = 0.75f;
    static std::vector<uint32_t> s_idleAnimFrames = {
        1, 6
    };
    static float s_moveAnimSpeed = 10.0f;
    static std::vector<uint32_t> s_moveAnimFrames = {
        11, 16, 21, 26
    };


    // Mapping for what pixel value to use depending on tile's temperature
    // *These go in order of the TileStateTemperature enum
    // NOTE: May need some adjustment at some point...
    static std::vector<PK_ubyte> s_tileTemperatureTexValues = {
        126, // mild
        91, // chilly
        55, // cold
        19, // freezing
        163, // warm
        199, // hot
        235 // burning
    };


    static std::chrono::time_point<std::chrono::high_resolution_clock> s_lastSend;
    // Used to start sending in sync with receiving world state
    // NOTE: This may not even fucking matter?
    static bool s_allowSend = false;

    // NOTE: May fuck up if getting message with outdated received position!
    // TODO: Skip update if received tile was not current local actual tile? ..or something like that..
    //  -> or some way to sync local and server position to get the actual correct area state!
    void World::OnMessageWorldState::onMessage(const GC_byte* data, size_t dataSize)
    {
        WorldObserver& observerRef = visualWorldRef._observer;
        const int dataWidth = (observerRef.observeRadius * 2) + 1;
        const size_t expectedDataSize = MESSAGE_REQUIRED_SIZE__WorldStateMsg;
        if (dataSize == expectedDataSize)
        {
            int32_t recvX = *(int32_t*)(data + MESSAGE_ENTRY_SIZE__header);
            int32_t recvY = *(int32_t*)(data + MESSAGE_ENTRY_SIZE__header + sizeof(int32_t));
            Debug::log("___TEST___recv coords: " + std::to_string(recvX) + ", " + std::to_string(recvY));
            observerRef.lastReceivedMapX = recvX;
            observerRef.lastReceivedMapY = recvY;

            visualWorldRef.shift(observerRef.lastReceivedMapX, observerRef.lastReceivedMapY);

            // NOTE: It looks fucked up if not updating all and shifting immediately here.. don't understant why tho..
            //  -> thats why the whol "triggering" thing isn't used atm
            // Trigger state update on next World::update
            //visualWorldRef.triggerStateUpdate(pReceivedState, receivedStateSize);
            const uint64_t* pReceivedState = (const uint64_t*)(data + MESSAGE_ENTRY_SIZE__header + sizeof(int32_t) * 2);
            const size_t receivedStateSize = dataSize - MESSAGE_ENTRY_SIZE__header;
            visualWorldRef.updateObservedArea(pReceivedState);
            visualWorldRef.moveTerrain();
            visualWorldRef.updateObjects();

            s_allowSend = true;
        }
        else
        {
            int offset = 0;
            Debug::log("___TEST___ERROR: msg too long. Found message types:");

            while (offset < dataSize)
            {
                uint32_t msgType = (uint32_t)(*(data + offset));
                Debug::log("___TEST___msg type: " + std::to_string(msgType) + " offset: " + std::to_string(offset));
                offset += expectedDataSize;
            }

            Debug::log("___TEST___world state response fatal error", Debug::MessageType::PK_FATAL_ERROR);
        }
    }


    World::World(
        pk::Scene& scene,
        pk::Transform* pCamTransform,
        int observeRadius,
        float tileVisualScale
    ) :
        _sceneRef(scene),
        _pCamTransform(pCamTransform),
        _tileVisualScale(tileVisualScale)
    {
        _observer.observeRadius = observeRadius;

        const int observeAreaWidth = _observer.observeRadius * 2 + 1;
        _tileDataSize = observeAreaWidth * observeAreaWidth * sizeof(uint64_t);
        _pTileData = new uint64_t[observeAreaWidth * observeAreaWidth];
        memset(_pTileData, 0, _tileDataSize);

        ResourceManager& resourceManager = Application::get()->getResourceManager();

        // init last send
        s_lastSend = std::chrono::high_resolution_clock::now();

        // Create terrain
        _terrainEntity = _sceneRef.createEntity();
        _sceneRef.createTransform(
            _terrainEntity,
            { 0, 0, 0 },
            { 0, 0, 0, 1 },
            { 1, 1, 1}
        );

        // Create initial terrain textures and material
        TextureSampler blendmapTexSampler(
            TextureSamplerFilterMode::PK_SAMPLER_FILTER_MODE_LINEAR,
            TextureSamplerAddressMode::PK_SAMPLER_ADDRESS_MODE_REPEAT,
            2
        );
        TextureSampler channelTexSampler(
            TextureSamplerFilterMode::PK_SAMPLER_FILTER_MODE_NEAR,
            TextureSamplerAddressMode::PK_SAMPLER_ADDRESS_MODE_REPEAT,
            2
        );

        // Create terrain blendmap
        int blendmapChannels = 4;
        _blendmapWidth = get_next_pow2(observeAreaWidth);
        size_t blendmapDataSize = (_blendmapWidth * _blendmapWidth) * blendmapChannels;
        PK_ubyte* pInitBlendmapData = new PK_ubyte[blendmapDataSize];
        memset(pInitBlendmapData, 0, blendmapDataSize);

        _pTerrainBlendmapImg = resourceManager.createImage(
            pInitBlendmapData,
            _blendmapWidth,
            _blendmapWidth,
            blendmapChannels
        );
        delete[] pInitBlendmapData;

        _pTerrainBlendmapTexture = resourceManager.createTexture(
            _pTerrainBlendmapImg->getResourceID(),
            blendmapTexSampler
        );

        // Create terrain "temperature texture" (single channel grayscale)
        size_t temperatureDataSize = _blendmapWidth * _blendmapWidth;
        PK_ubyte* pInitTemperatureData = new PK_ubyte[temperatureDataSize];
        memset(pInitTemperatureData, 0, temperatureDataSize);

        _pTerrainTemperatureImg = resourceManager.createImage(
            pInitTemperatureData,
            _blendmapWidth,
            _blendmapWidth,
            1
        );
        delete[] pInitTemperatureData;

        _pTerrainTemperatureTexture = resourceManager.createTexture(
            _pTerrainTemperatureImg->getResourceID(),
            blendmapTexSampler
        );


        // Terrain channel textures
        ImageData* pImgChannel0 = resourceManager.loadImage("assets/textures/deadland.png");
        ImageData* pImgChannel1 = resourceManager.loadImage("assets/textures/water.png");
        ImageData* pImgChannel2 = resourceManager.loadImage("assets/textures/rock.png");
        ImageData* pImgChannel3 = resourceManager.loadImage("assets/textures/grass.png");
        ImageData* pImgChannel4 = resourceManager.loadImage("assets/textures/dunes.png");

        Texture* pTerrainTex0 = resourceManager.createTexture(
            pImgChannel0->getResourceID(),
            channelTexSampler
        );
        Texture* pTerrainTex1 = resourceManager.createTexture(
            pImgChannel1->getResourceID(),
            channelTexSampler
        );
        Texture* pTerrainTex2 = resourceManager.createTexture(
            pImgChannel2->getResourceID(),
            channelTexSampler
        );
        Texture* pTerrainTex3 = resourceManager.createTexture(
            pImgChannel3->getResourceID(),
            channelTexSampler
        );
        Texture* pTerrainTex4 = resourceManager.createTexture(
            pImgChannel4->getResourceID(),
            channelTexSampler
        );
        TerrainMaterial* pTerrainMaterial = resourceManager.createTerrainMaterial(
            {
                pTerrainTex0->getResourceID(),
                pTerrainTex1->getResourceID(),
                pTerrainTex2->getResourceID(),
                pTerrainTex3->getResourceID(),
                pTerrainTex4->getResourceID()
            },
            _pTerrainBlendmapTexture->getResourceID(),
            _pTerrainTemperatureTexture->getResourceID()
        );

        std::vector<float> initialHeightmap(observeAreaWidth * observeAreaWidth, 0.0f);
        _pTerrainMesh = resourceManager.createTerrainMesh(
            initialHeightmap,
            _tileVisualScale,
            pTerrainMaterial->getResourceID()
        );

        TerrainRenderable* pTerrainRenderable = _sceneRef.createTerrainRenderable(
            _terrainEntity,
            _pTerrainMesh->getResourceID(),
            pTerrainMaterial->getResourceID(),
            initialHeightmap,
            _tileVisualScale
        );

        // Create tile objects at first as "blank"
        //  -> we configure these eventually, when we fetch world state from server
        _visibleObjects.reserve(observeAreaWidth * observeAreaWidth);

        Model* pDefaultStaticModel = objects::ObjectInfoLib::get_default_static_model();
        Model* pDefaultRiggedModel = objects::ObjectInfoLib::get_default_rigged_model();
        const Mesh* pDefaultRiggedMesh = pDefaultRiggedModel->getMesh(0);

        Animation* pDefaultAnimation = resourceManager.createAnimation(
            pDefaultRiggedMesh->getBindPose(),
            pDefaultRiggedMesh->getAnimPoses()
        );

        for (int y = 0; y < observeAreaWidth; ++y)
        {
            for (int x = 0; x < observeAreaWidth; ++x)
            {
                entityID_t visualObjEntity = _sceneRef.createEntity();

                const vec3 originalGridPos(x * _tileVisualScale, 0, y * _tileVisualScale);
                Transform* pTransform = _sceneRef.createTransform(
                    visualObjEntity,
                    originalGridPos,
                    { 0, 0, 0, 1},
                    { 1.0f, 1.0f, 1.0f }
                    //{ _tileVisualScale, _tileVisualScale, _tileVisualScale },
                );
                pTransform->setActive(true);

                Static3DRenderable* pStaticRenderable = _sceneRef.createStatic3DRenderable(
                    visualObjEntity,
                    pDefaultStaticModel->getMesh(0)->getResourceID()
                );
                pStaticRenderable->setActive(false);

                // NOTE: ISSUE!
                // This assumes we always use same skeleton
                // -> requires that the model's skeleton config is also always the same..
                SkinnedRenderable* pSkinnedRenderable = _sceneRef.createSkinnedRenderable(
                    visualObjEntity,
                    pDefaultRiggedModel->getResourceID(),
                    pDefaultRiggedMesh->getResourceID()
                );
                pSkinnedRenderable->setActive(false);

                _sceneRef.createAnimationData(
                    visualObjEntity,
                    ((Resource*)pDefaultAnimation)->getResourceID(),
                    AnimationMode::PK_ANIMATION_MODE_REPEAT,
                    s_moveAnimSpeed,
                    s_moveAnimFrames
                );


                objects::VisualObject visualObj(
                    *this,
                    visualObjEntity,
                    pStaticRenderable,
                    pSkinnedRenderable,
                    originalGridPos
                );
                _tileObjects.push_back(visualObj);

                _sceneRef.addChild(_terrainEntity, visualObjEntity);

                /*
                uint32_t tileEntity = _sceneRef.createEntity();

                TerrainTileRenderable* tileRenderable = new TerrainTileRenderable(
                        x * _tileVisualScale, y * _tileVisualScale,
                        x, y,
                        _tileVisualScale
                        );

                Sprite3DRenderable* effectRenderable = new Sprite3DRenderable(
                        { x * _tileVisualScale, 0, y * _tileVisualScale },
                        { _tileVisualScale, _tileVisualScale },
                        nullptr
                        );
                Sprite3DRenderable* objectRenderable = new Sprite3DRenderable(
                        { x * _tileVisualScale, 0, y * _tileVisualScale },
                        { _tileVisualScale, _tileVisualScale },
                        nullptr
                        );

                _sceneRef.addComponent(tileEntity, tileRenderable);
                _sceneRef.addComponent(tileEntity, effectRenderable);
                _sceneRef.addComponent(tileEntity, objectRenderable);

                VisualTile t(scene, *this, tileEntity, tileRenderable, effectRenderable, objectRenderable);

                uint64_t initialState = 0x0;
                _tileData.push_back(std::make_pair(initialState, t));
                */
            }
        }

        // Create "tile progressions/movements/anims table"
        const size_t tileCount = observeAreaWidth * observeAreaWidth;
        _tileAnimStates.resize(tileCount);
        for (int i = 0; i < tileCount; ++i)
        {
            TileAnimation tileAnim({ 0, 0, 0 }, 0, 1, 0.0f);
            _tileAnimStates[i] = tileAnim;
        }
    }

    World::~World()
    {
        delete[] _pTileData;
    }

    // ..quite shit and inefficient
    void World::updateObservedArea(const uint64_t* mapState)
    {
        _shouldUpdateLocalState = false;
        const int observeAreaWidth = _observer.observeRadius * 2 + 1;

        // Update terrain heights and blendmap
        const size_t bufferStride = sizeof(float) * 8; // pos(3) + normal(3) + uv(2) = 8 floats
        Buffer* pBuffer = _pTerrainMesh->accessVertexBuffer();
        for (int y = 0; y < observeAreaWidth; ++y)
        {
            for (int x = 0; x < observeAreaWidth; ++x)
            {
                const int tileIndex = x + y * observeAreaWidth;
                _pTileData[tileIndex] = mapState[tileIndex];
                uint64_t tileState = mapState[tileIndex];

                float height = (float)(get_tile_terrelevation(tileState));
                PK_ubyte tileType = get_tile_terrtype(tileState);
                PK_ubyte tileTemperature = get_tile_temperature(tileState);

                size_t vertexYBufPos = sizeof(float) + (x + y * observeAreaWidth) * bufferStride;
                pBuffer->update(
                    &height,
                    vertexYBufPos,
                    sizeof(float)
                );

				// Figure out new normal (quite shit and unpercise way, but it looks fine for now..)
				float left = 0;
				float right = 0;
				float down = 0;
				float up = 0;

				if (x - 1 >= 0)
                    left = (float)(get_tile_terrelevation(mapState[(x-1) + y * observeAreaWidth]));

				if (x + 1 < observeAreaWidth)
                    right = (float)(get_tile_terrelevation(mapState[(x+1) + y * observeAreaWidth]));

				if (y + 1 < observeAreaWidth)
                    up = (float)(get_tile_terrelevation(mapState[x + (y+1) * observeAreaWidth]));

				if (y - 1 >= 0)
                    down = (float)(get_tile_terrelevation(mapState[x + (y-1) * observeAreaWidth]));

				vec3 normal((left - right), 15.0f, (down - up)); // this is fucking dumb...
				normal = normal.normalize();

                size_t normalBufPos = sizeof(float) * 3 + (x + y * observeAreaWidth) * bufferStride;
                pBuffer->update(
                    &normal,
                    normalBufPos,
                    sizeof(vec3)
                );


                updateBlendmapData(tileType, x, y, tileTemperature);

                // NOTE: Deprecated below?
                // TODO: look how thats done atm!
                // Also need to update objects' colliders scales depending on obj type here
                // since "updateObjects()" happens in lateUpdate so.. fucks up if done there...
                /*
                PK_ubyte tileObj = get_tile_thingid(tileState);
                if (tileObj)
                {
                    VisualObject& visualObj = _tileObjects[tileIndex];
                    Transform* pColliderTransform = (Transform*)_sceneRef.getComponent(
                        visualObj.getColliderEntity(),
                        ComponentType::PK_TRANSFORM
                    );
                    pColliderTransform->setScale(VisualObject::s_colliderSizes[tileObj]);
                }
                */
            }
        }
        _pTerrainBlendmapTexture->update(
            (void*)_pTerrainBlendmapImg->getData(),
            _pTerrainBlendmapImg->getSize(),
            _pTerrainBlendmapImg->getWidth(),
            _pTerrainBlendmapImg->getHeight(),
            4 // atm needed for gl fuckery..
        );
        _pTerrainTemperatureTexture->update(
            (void*)_pTerrainTemperatureImg->getData(),
            _pTerrainTemperatureImg->getSize(),
            _pTerrainTemperatureImg->getWidth(),
            _pTerrainTemperatureImg->getHeight(),
            5 // atm needed for gl fuckery..
        );
    }

    void World::updateObjects()
    {
        const int observeAreaWidth = _observer.observeRadius * 2 + 1;
        for (int y = 0; y < observeAreaWidth; ++y)
        {
            for (int x = 0; x < observeAreaWidth; ++x)
            {
                const int tileIndex = x + y * observeAreaWidth;
                uint64_t tileState = _pTileData[tileIndex];

                PK_ubyte tileEffect = get_tile_effect(tileState);
                PK_ubyte tileObject = get_tile_thingid(tileState);
                PK_ubyte tileAction = get_tile_action(tileState);
                PK_ubyte tileFacingDirection = get_tile_facingdir(tileState);


                // Reset movements if no action, even in case we didn't have any object here
                // so shifted anims don't look fucked
                if (!tileAction)
                    _tileAnimStates[tileIndex].reset();

                VisualObject& obj = _tileObjects[tileIndex];
                if (tileObject)
                {
                    const float worldPosX = x * _tileVisualScale;
                    const float worldPosZ = y * _tileVisualScale;
                    obj.show(
                        &_sceneRef,
                        tileObject,
                        tileAction,
                        tileFacingDirection,
                        *objects::ObjectInfoLib::get(tileObject),
                        *objects::ObjectInfoLib::get_visual(tileObject),
                        worldPosX,
                        worldPosZ,
                        _tileAnimStates[tileIndex].pos
                    );
                    _visibleObjects.emplace_back(tileIndex);
                }
                else
                {
                    obj.hide(&_sceneRef);
                }
            }
        }
    }

    void World::moveTerrain()
    {
        // Move terrain if current tile changed
        const int observeAreaWidth = _observer.observeRadius * 2 + 1;
        float observeTileX = (float)_observer.lastReceivedMapX;
        float observeTileY = (float)_observer.lastReceivedMapY;
        const float halfTileWidth = _tileVisualScale * 0.5f;
        float halfTerrainWorldWidth = ((float)observeAreaWidth) * halfTileWidth;

        const float terrainWorldX = ((observeTileX * _tileVisualScale) - halfTerrainWorldWidth);
        const float terrainWorldZ = ((observeTileY * _tileVisualScale) - halfTerrainWorldWidth);

        Transform* pTerrainTransform = (Transform*)_sceneRef.getComponent(_terrainEntity, ComponentType::PK_TRANSFORM);
        mat4& tMat = pTerrainTransform->accessTransformationMatrix();
        // Also need to add little offset cuz using vertices as "tiles"!
        tMat[0 + 3 * 4] = terrainWorldX + halfTileWidth;
        tMat[2 + 3 * 4] = terrainWorldZ + halfTileWidth;
    }

    // Shifts "movements"-table, if moved camera, to make it look smooth
    void World::shift(int32_t tileX, int32_t tileY)
    {
        const int observeAreaWidth = _observer.observeRadius * 2 + 1;

        //Animation* tempAnim = new Animation({ 0 }, 1.0f);
        // moving right(default) (shift left)
        int startX = tileX > _prevTileX ? observeAreaWidth - 1 : 0;
        int startY = tileY > _prevTileY ? observeAreaWidth - 1 : 0;
        bool incrX = tileX < _prevTileX;
        bool incrY = !(tileY > _prevTileY);

        int shiftCountX = std::abs(tileX - _prevTileX);
        int shiftCountY = std::abs(tileY - _prevTileY);

        // Horizontal shifting
        if (tileX != _prevTileX)
        {
            for (int y = 0; y < observeAreaWidth; ++y)
            {
                for (int shiftCount = 0; shiftCount < shiftCountX; shiftCount++)
                {
                    TileAnimation prevAnim = _tileAnimStates[0 + y * observeAreaWidth];
                    prevAnim.reset();

                    for (int x = startX; incrX ? x < observeAreaWidth : x >= 0; incrX ? ++x : --x)
                    {
                        const int tileIndex = x + y * observeAreaWidth;
                        TileAnimation currentAnim = _tileAnimStates[tileIndex];
                        _tileAnimStates[tileIndex] = prevAnim;

                        prevAnim = currentAnim;
                    }
                }
            }
        }
        // Vertical shifting
        if (tileY != _prevTileY)
        {
            for (int x = 0; x < observeAreaWidth; ++x)
            {
                for (int shiftCount = 0; shiftCount < shiftCountY; shiftCount++)
                {
                    TileAnimation prevAnim = _tileAnimStates[x + 0 * observeAreaWidth];
                    prevAnim.reset();
                    for (int y = startY; incrY ? y < observeAreaWidth : y >= 0; incrY ? ++y : --y)
                    {
                        const int tileIndex = x + y * observeAreaWidth;
                        TileAnimation currentAnim = _tileAnimStates[tileIndex];
                        _tileAnimStates[tileIndex] = prevAnim;

                        prevAnim = currentAnim;
                    }
                }
            }
        }

        // Save previous tile pos
        _prevTileX = tileX;
        _prevTileY = tileY;
    }

    void World::update(float worldX, float worldZ)
    {
        _worldX = worldX;
        _worldZ = worldZ;

        // Calc the "map pos" according to "visual float pos"(this should be camera's pivot point, if rts style camera)
        //int32_t tileX = (int32_t)std::floor((_worldX - (float)_observer.observeRadius * _tileVisualScale) / _tileVisualScale);
        //int32_t tileY = (int32_t)std::floor((_worldZ - (float)_observer.observeRadius * _tileVisualScale) / _tileVisualScale);

        worldToTileCoords(_worldX, _worldZ, _tileX, _tileY);


        // Can receive world state and send location only after logging in
        Client* pClient = Client::get_instance();
        if (pClient->isConnected() && pClient->user.isLoggedIn)
        {
            if (!_initialized)
            {
                pClient->addOnMessageEvent(
                    MESSAGE_TYPE__WorldState,
                    new OnMessageWorldState(*this)
                );
                _initialized = true;
            }
            else
            {

                // Send current observing position if tile had changed
                std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> elapsedSinceLastSend = currentTime - s_lastSend;
                if (s_allowSend)
                {
                    if (elapsedSinceLastSend.count() >= 0.3f)
                    {
                        if (_tileX != _observer.lastReceivedMapX || _tileY != _observer.lastReceivedMapY)
                        {
                            _observer.requestedMapX = _tileX;
                            _observer.requestedMapY = _tileY;
                            Client::get_instance()->send(
                                (int32_t)MESSAGE_TYPE__UpdateObserverProperties,
                                {
                                    {
                                        (PK_byte*)(&_observer.requestedMapX),
                                        sizeof(int32_t), sizeof(int32_t)
                                    },
                                    {
                                        (PK_byte*)(&_observer.requestedMapY),
                                        sizeof(int32_t), sizeof(int32_t)
                                    },
                                    {
                                        (PK_byte*)(&_observer.observeRadius),
                                        sizeof(int32_t), sizeof(int32_t)
                                    }
                                }
                            );
                            s_lastSend = std::chrono::high_resolution_clock::now();
                        }
                    }
                }
            }
        }


        // Visuals aren't in sync and looks fucked if using "triggering"
        // system to update
        /*
        if (_shouldUpdateLocalState)
        {
            shift(_observer.lastReceivedMapX, _observer.lastReceivedMapY);
            updateObservedArea(_pTileData);
        }*/

        // Update cam facing direction
        // NOTE: Atm gets fucked since _pCamTransform ptr is out of date
        // due to propable transform component pool resizing!
        // TODO: Fix this!
        vec3 camForward = _pCamTransform->forward();
        vec2 camDirVec(camForward.x, camForward.z);
        camDirVec.normalize();
        float angle = std::atan2(camDirVec.y, camDirVec.x);
        const float base = 8.0f;
        const float displace = M_PI / base;
        float fDir = (angle + displace + M_PI * 0.5f) / (M_PI / (base * 0.5f));
        if (fDir < 0.0f)
            fDir = base + fDir;

        _cameraDirection = (int)std::floor(fDir);

        // Atm object update should be done immediately after state update and shifting
        // has been done by on message event
        //updateObjects();
        //updateSprites();

        _visibleObjects.clear();
    }

    void World::addFaction(const Faction& faction)
    {
        std::string factionName(faction.getName());
        std::unordered_map<std::string, Faction>::iterator it = _factions.find(factionName);
        if (it == _factions.end())
        {
            Debug::log("___TEST___adding faction: " + factionName + " to world");
            _factions.insert(std::make_pair(factionName, faction));
        }
        else
        {
            Debug::log(
                "Attempted to add faction: " + factionName +
                " but faction with this name is already added",
                Debug::MessageType::PK_ERROR
            );
        }
    }
    void World::updateFaction(const Faction& faction)
    {
        std::string factionName(faction.getName());
        std::unordered_map<std::string, Faction>::iterator it = _factions.find(factionName);
        if (it != _factions.end())
        {
            Debug::log("___TEST___updating faction: " + factionName);
            it->second = faction;
        }
        else
        {
            Debug::log(
                "Attempted to update faction: " + factionName +
                " but faction with this name didn't exist",
                Debug::MessageType::PK_ERROR
            );
        }
    }

    void World::updateFactionList(const std::string& factionName, const Faction& faction)
    {
        _factions[factionName] = faction;
        Debug::log("___TEST___UPDATED FACTION: " + factionName + " faction count = " + std::to_string(_factions.size()));
    }

    Faction World::getFaction(const std::string& factionName) const
    {
        std::unordered_map<std::string, Faction>::const_iterator it = _factions.find(factionName);
        if (it != _factions.end())
        {
            return it->second;
        }
        Debug::log("Failed to get faction: " + factionName + " from world");
        return NULL_FACTION;
    }

    bool World::factionExists(const std::string& factionName) const
    {
        return _factions.find(factionName) != _factions.end();
    }

    static float get_triangle_height_barycentric(
            const vec3& p1,
            const vec3& p2,
            const vec3& p3,
            const vec2& pos
            )
    {
        float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
        float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
        float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
        float l3 = 1.0f - l1 - l2;
        return l1 * p1.y + l2 * p2.y + l3 * p3.y;
    }

    float World::getTerrainHeight(float worldX, float worldZ) const
	{
		// Pos relative to terrain
        Transform* pTerrainTransform = (Transform*)_sceneRef.getComponent(
            _terrainEntity,
            ComponentType::PK_TRANSFORM
        );
        const mat4& tMat = pTerrainTransform->getTransformationMatrix();

		const float terrainWorldX = tMat[0 + 3 * 4];
		const float terrainWorldZ = tMat[2 + 3 * 4];
		const int verticesPerRow = _observer.observeRadius * 2 + 1;

		float terrainX = worldX - terrainWorldX;
		float terrainZ = worldZ - terrainWorldZ;

		int gridX = (int)std::floor(terrainX / _tileVisualScale);
		int gridZ = (int)std::floor(terrainZ / _tileVisualScale);

		if (gridX < 0 || gridX + 1 >= verticesPerRow || gridZ < 0 || gridZ + 1 >= verticesPerRow)
		{
			return 0.0f;
		}

		// Coordinates in relation to the current tile, in range 0 to 1
		float tileSpaceX = std::fmod(terrainX, _tileVisualScale) / _tileVisualScale;
		float tileSpaceZ = std::fmod(terrainZ, _tileVisualScale) / _tileVisualScale;

		// Check which triangle of the tile we are standing on..
        // Could be optimized by having local heightmap separately from _pTileData?
		if (tileSpaceX <= tileSpaceZ) {
			return get_triangle_height_barycentric(
				vec3(0, get_tile_terrelevation(_pTileData[gridX + gridZ * verticesPerRow]), 0),
			    vec3(0, get_tile_terrelevation(_pTileData[gridX + (gridZ + 1) * verticesPerRow]), 1),
			    vec3(1, get_tile_terrelevation(_pTileData[(gridX + 1) + (gridZ + 1) * verticesPerRow]), 1),
			    vec2(tileSpaceX, tileSpaceZ));
		}
		else {
			return get_triangle_height_barycentric(
				vec3(0, get_tile_terrelevation(_pTileData[gridX + gridZ * verticesPerRow]), 0),
				vec3(1, get_tile_terrelevation(_pTileData[(gridX + 1) + (gridZ + 1) * verticesPerRow]), 1),
				vec3(1, get_tile_terrelevation(_pTileData[(gridX + 1) + gridZ * verticesPerRow]), 0),
				vec2(tileSpaceX, tileSpaceZ));
		}
	}

    void World::worldToTileCoords(float x, float z, int& outTileX, int& outTileY)
    {
        // Need to add little offset cuz using vertices as "tiles"
        float halfTileWidth = _tileVisualScale * 0.5f;
        float displacedWorldX = x + halfTileWidth;
        float displacedWorldZ = z + halfTileWidth;
        outTileX = (int)std::floor(displacedWorldX / _tileVisualScale);
        outTileY = (int)std::floor(displacedWorldZ / _tileVisualScale);
    }

    void World::updateBlendmapData(PK_ubyte tileType, int x, int y, GC_ubyte temperature)
    {
        // channels follow TileStateTerrType enum in following order
        // black =  0
        // red =    1
        // green =  2
        // blue =   3
        // alpha =  4
        // First channel using "black" so need do a bit differently
        if (tileType == 0)
        {
            _pTerrainBlendmapImg->setColorAt_UNSAFE(x, y, 0, 0, 0, 0);
        }
        else
        {
            const int r = tileType == 1 ? 255 : 0;
            const int g = tileType == 2 ? 255 : 0;
            const int b = tileType == 3 ? 255 : 0;
            const int a = tileType == 4 ? 255 : 0;
            _pTerrainBlendmapImg->setColorAt_UNSAFE(x, y, r, g, b, a);
        }

        // atm just testing temperature data thing...
        const PK_ubyte temperatureTexValue = s_tileTemperatureTexValues[temperature];
        _pTerrainTemperatureImg->setColorAt_UNSAFE(x, y, temperatureTexValue);
    }

    void World::updateBlendmapData(PK_ubyte tileType, int pixelIndex)
    {
        const int r = tileType == 1 ? 255 : 0;
        const int g = tileType == 2 ? 255 : 0;
        const int b = tileType == 3 ? 255 : 0;
        const int a = tileType == 4 ? 255 : 0;

        _pTerrainBlendmapImg->setColorAt_UNSAFE(pixelIndex, r, g, b, a);
    }

    void World::triggerStateUpdate(const GC_byte* pNewState, size_t stateSize)
    {
        if (stateSize != _tileDataSize)
        {
            Debug::log(
                "@World::setAreaState "
                "received state size: " + std::to_string(stateSize) + " "
                "wasn't same as required size: " + std::to_string(_tileDataSize),
                Debug::MessageType::PK_FATAL_ERROR
            );
            return;
        }
        memcpy(_pTileData, pNewState, _tileDataSize);
        _shouldUpdateLocalState = true;
    }

    uint64_t World::getTile(int32_t tileX, int32_t tileY) const
    {
        uint32_t observeAreaWidth = _observer.observeRadius * 2 + 1;
        if (tileX < 0 || tileX >= observeAreaWidth || tileY < 0 || tileY >= observeAreaWidth)
            return 0;
        return *(_pTileData + (tileX + tileY * observeAreaWidth));
    }
}
