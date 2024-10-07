#include <algorithm>
#include <chrono>
#include <cstring>
#include <string>

#include "Objects.h"
#include "World.h"
#include "Tile.h"
#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/messages/Message.h"

#include <chrono>

using namespace pk;
using namespace pk::web;
using namespace gamecommon;

using namespace net;


namespace world
{
    using namespace objects;


    static std::chrono::time_point<std::chrono::high_resolution_clock> s_lastSend;
    static bool s_TEST_skipUpdate = false;
    static bool s_TEST_allowSend = false;

    void World::OnMessageWorldState::onMessage(const GC_byte* data, size_t dataSize)
    {
        WorldObserver& observerRef = visualWorldRef._observer;
        const int dataWidth = (observerRef.observeRadius * 2) + 1;
        const size_t expectedDataSize = MESSAGE_ENTRY_SIZE__header + (dataWidth * dataWidth) * sizeof(uint64_t);
        // WARNING TESTING:
        // if received multiple times use only "first message"
        if (dataSize == expectedDataSize)
        {
            observerRef.lastReceivedMapX = visualWorldRef._observer.requestedMapX;
            observerRef.lastReceivedMapY = visualWorldRef._observer.requestedMapY;

            /*
            if (s_TEST_skipUpdate)
            {
                Debug::log("___TEST___skipping update");
                s_TEST_skipUpdate = false;
                return;
            }
            */

            visualWorldRef.shift(observerRef.lastReceivedMapX, observerRef.lastReceivedMapY);

            // NOTE: It looks fucked up if not updating all and shifting immediately here.. don't understant why tho..
            //  -> thats why the whol "triggering" thing isn't used atm
            // Trigger state update on next World::update
            //visualWorldRef.triggerStateUpdate(pReceivedState, receivedStateSize);

            const uint64_t* pReceivedState = (const uint64_t*)(data + MESSAGE_ENTRY_SIZE__header);
            const size_t receivedStateSize = dataSize - MESSAGE_ENTRY_SIZE__header;
            visualWorldRef.updateObservedArea(pReceivedState);
            visualWorldRef.moveTerrain();
            visualWorldRef.updateObjects();

            s_TEST_allowSend = true;
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


    World::World(pk::Scene& scene, pk::Transform* pCamTransform, int observeRadius) :
        _sceneRef(scene),
        _pCamTransform(pCamTransform)
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
        // Terrain channel textures
        ImageData* pImgChannel0 = resourceManager.loadImage("assets/textures/deadland.png");
        ImageData* pImgChannel1 = resourceManager.loadImage("assets/textures/water.png");
        ImageData* pImgChannel2 = resourceManager.loadImage("assets/textures/snow.png");
        ImageData* pImgChannel3 = resourceManager.loadImage("assets/textures/rock.png");
        ImageData* pImgChannel4 = resourceManager.loadImage("assets/textures/grass.png");

        Texture_new* pTerrainTex0 = resourceManager.createTexture(
            pImgChannel0->getResourceID(),
            channelTexSampler
        );
        Texture_new* pTerrainTex1 = resourceManager.createTexture(
            pImgChannel1->getResourceID(),
            channelTexSampler
        );
        Texture_new* pTerrainTex2 = resourceManager.createTexture(
            pImgChannel2->getResourceID(),
            channelTexSampler
        );
        Texture_new* pTerrainTex3 = resourceManager.createTexture(
            pImgChannel3->getResourceID(),
            channelTexSampler
        );
        Texture_new* pTerrainTex4 = resourceManager.createTexture(
            pImgChannel4->getResourceID(),
            channelTexSampler
        );
        Material* pTerrainMaterial = resourceManager.createMaterial(
            {
                pTerrainTex0->getResourceID(),
                pTerrainTex1->getResourceID(),
                pTerrainTex2->getResourceID(),
                pTerrainTex3->getResourceID(),
                pTerrainTex4->getResourceID()
            },
            0,
            0.0f,
            0.0f,
            _pTerrainBlendmapTexture->getResourceID()
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
        TextureSampler defaultObjTextureSampler;
        ImageData* pDefaultObjTexImage = resourceManager.loadImage("assets/textures/default.jpg");
        Texture_new* pDefaultObjTexture = resourceManager.createTexture(
            pDefaultObjTexImage->getResourceID(),
            defaultObjTextureSampler
        );
        Material* pDefaultObjMaterial = resourceManager.createMaterial(
            { pDefaultObjTexture->getResourceID() }
        );
        Model* pDefaultObjModel = resourceManager.loadModel(
            "assets/models/Arrow.glb",
            pDefaultObjMaterial->getResourceID()
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
                    pDefaultObjModel->getMesh(0)->getResourceID()
                );

                pStaticRenderable->setActive(false);

                objects::VisualObject visualObj(
                    *this,
                    visualObjEntity,
                    pStaticRenderable,
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
            //Animation* anim = new Animation({ 0 }, 4.0f);
            //anim->enableLooping(true);
            //anim->play();
            TileAnimation tileAnim({ 0, 0, 0 }/*, anim*/);
            _tileAnimStates[i] = tileAnim;
            //scene.addSystem((pk::System*)anim);
        }

        // Add OnMessageEvents
        // to update world state
        //Client::get_instance()->addOnMessageEvent(MESSAGE_TYPE__WorldState, new OnMessageWorldState(*this));
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
        Buffer* pBuffer = _pTerrainMesh->accessVertexBuffer_DANGER(0);
        for (int y = 0; y < observeAreaWidth; ++y)
        {
            for (int x = 0; x < observeAreaWidth; ++x)
            {
                const int tileIndex = x + y * observeAreaWidth;
                _pTileData[tileIndex] = mapState[tileIndex];
                uint64_t tileState = mapState[tileIndex];

                // Set vertex pos.y (height)
                float height = (float)(get_tile_terrelevation(tileState));
                // No idea wtf is this..
                //const float max = 15.0f;
                //if (height - max >= 0.0f)
                //    height -= max;

                size_t vertexYBufPos = sizeof(float) + (x + y * observeAreaWidth) * (sizeof(float) * 8);
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

				vec3 normal((left - right), 1.0f, (down - up)); // this is fucking dumb...
				normal = normal.normalize();

                size_t normalBufPos = sizeof(float) * 3 + (x + y * observeAreaWidth) * (sizeof(float) * 8);
                pBuffer->update(
                    &normal,
                    normalBufPos,
                    sizeof(vec3)
                );

                PK_ubyte tileType = get_tile_terrtype(tileState);
                updateBlendmapData(tileType, x, y);
            }
        }

        _pTerrainBlendmapTexture->update(
            (void*)_pTerrainBlendmapImg->getData(),
            _pTerrainBlendmapImg->getSize(),
            _pTerrainBlendmapImg->getWidth(),
            _pTerrainBlendmapImg->getHeight(),
            4 // atm needed for gl fuckery..
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

                PK_ubyte tileEffect = get_tile_terreffect(tileState);
                PK_ubyte tileObject = get_tile_thingid(tileState);
                PK_ubyte tileAction = get_tile_action(tileState);
                PK_ubyte tileFacingDirection = get_tile_facingdir(tileState);


                // Reset movements if no action, even in case we didn't have any object here
                if (!tileAction)
                {
                    //if (_tileAnimStates[tileIndex].pos.x != 0.0f)
                    //    Debug::log("___TEST___anim reset at: " + std::to_string(x) + ", " + std::to_string(y));
                    _tileAnimStates[tileIndex].reset();
                }
                else
                {
                    //Debug::log("___TEST___updating anim at: " + std::to_string(x) + ", " + std::to_string(y));
                }

                // Test displaying some object
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
                        *objects::ObjectInfoLib::getVisual(tileObject),
                        worldPosX,
                        worldPosZ,
                        _tileAnimStates[tileIndex].pos
                    );
                }
                else
                {
                    obj.hide(&_sceneRef);
                }


                /*
                // TODO: Delete below
                //  -> "visual tiles" and "tile renderables" don't exist anymore
                //VisualTile& visualTile = _tileData[tileIndex].second;
                //TerrainTileRenderable* tileRenderable = visualTile.renderable_tile;

                // NOTE: May not work if component pools resized
                Static3DRenderable* pTileObjTransform = _tileObjects[tileIndex];

                const float spriteWorldX = ((float)tileRenderable->worldX + _tileVisualScale * 0.5f);
                const float spriteWorldZ = ((float)tileRenderable->worldZ + _tileVisualScale * 0.5f);

                Sprite3DRenderable* tileEffectSprite = _tileData[tileIndex].second.renderable_effect;
                objects::VisualObject& visualObject = _tileData[tileIndex].second.visualObject;

                // * Currently no effects exists yet!
                tileEffectSprite->setActive(false);

                // Reset movements if no action, even in case we didn't have any object here
                if (!tileAction)
                    _tileAnimStates[tileIndex].reset();

                // Set tile object sprite
                if (tileObject)
                {
                    int objDir = (int)get_tile_facingdir(tileState);
                    if (tileObject < objects::ObjectInfoLib::get_size())
                    {
                        visualObject.show(
                            tileObject,
                            tileAction,
                            objDir,
                            _cameraDirection,
                            *objects::ObjectInfoLib::get(tileObject),
                            *objects::ObjectInfoLib::getVisual(tileObject),
                            spriteWorldX,
                            spriteWorldZ,
                            _tileAnimStates[tileIndex].anim,
                            _tileAnimStates[tileIndex].pos
                        );
                    }
                    else
                    {
                        Debug::log(
                            "Failed to find client side info for object of type: " +
                            std::to_string(tileObject),
                            Debug::MessageType::PK_ERROR
                        );
                    }
                }
                else
                {
                    visualObject.hide();
                    //_tileAnimStates[tileIndex].anim->reset();
                }
                */
            }
        }
    }

    void World::moveTerrain()
    {
        // Move terrain if current tile changed
        // NOTE: Theres still something wonky how the grid moves.. feels something like rounding error, but not sure..
        // NOTE: These should actually be the last received coords, NOT requested coords?
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

    // Updates tile sprites
    // * Has to be done after updating terrain heights and very frequently to not look funny..
    /*
    void World::updateSprites()
    {
        const int observeAreaWidth = _observer.observeRadius * 2 + 1;
        for (int y = 0; y < observeAreaWidth; ++y)
        {
            for (int x = 0; x < observeAreaWidth; ++x)
            {
                const int tileIndex = x + y * observeAreaWidth;
                uint64_t tileState = _tileData[tileIndex].first;

                PK_ubyte tileEffect = get_tile_terreffect(tileState);
                PK_ubyte tileObject = get_tile_thingid(tileState);
                PK_ubyte tileAction = get_tile_action(tileState);

                VisualTile& visualTile = _tileData[tileIndex].second;
                TerrainTileRenderable* tileRenderable = visualTile.renderable_tile;

                const float spriteWorldX = ((float)tileRenderable->worldX + _tileVisualScale * 0.5f);
                const float spriteWorldZ = ((float)tileRenderable->worldZ + _tileVisualScale * 0.5f);

                Sprite3DRenderable* tileEffectSprite = _tileData[tileIndex].second.renderable_effect;
                objects::VisualObject& visualObject = _tileData[tileIndex].second.visualObject;

                // * Currently no effects exists yet!
                tileEffectSprite->setActive(false);

                // Reset movements if no action, even in case we didn't have any object here
                if (!tileAction)
                    _tileAnimStates[tileIndex].reset();

                // Set tile object sprite
                if (tileObject)
                {
                    int objDir = (int)get_tile_facingdir(tileState);
                    if (tileObject < objects::ObjectInfoLib::get_size())
                    {
                        visualObject.show(
                            tileObject,
                            tileAction,
                            objDir,
                            _cameraDirection,
                            *objects::ObjectInfoLib::get(tileObject),
                            *objects::ObjectInfoLib::getVisual(tileObject),
                            spriteWorldX,
                            spriteWorldZ,
                            _tileAnimStates[tileIndex].anim,
                            _tileAnimStates[tileIndex].pos
                        );
                    }
                    else
                    {
                        Debug::log(
                            "Failed to find client side info for object of type: " +
                            std::to_string(tileObject),
                            Debug::MessageType::PK_ERROR
                        );
                    }
                }
                else
                {
                    visualObject.hide();
                    //_tileAnimStates[tileIndex].anim->reset();
                }
            }
        }
    }
    */

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
                    pk::vec3 prevPos = _tileAnimStates[0 + y * observeAreaWidth].pos;
                    //Animation* prevAnim = _tileAnimStates[0 + y * observeAreaWidth].anim;
                    //prevAnim->reset();
                    for (int x = startX; incrX ? x < observeAreaWidth : x >= 0; incrX ? ++x : --x)
                    {
                        const int tileIndex = x + y * observeAreaWidth;
                        pk::vec3 currentPos = _tileAnimStates[tileIndex].pos;
                        //Animation* currentAnim = _tileAnimStates[tileIndex].anim;
                        //tempAnim->copyStateFrom(*currentAnim);
                        _tileAnimStates[tileIndex].pos = prevPos;
                        //_tileAnimStates[tileIndex].anim->copyStateFrom(*prevAnim);

                        prevPos = currentPos;
                        //prevAnim->copyStateFrom(*tempAnim);
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
                    pk::vec3 prevPos = _tileAnimStates[x + 0 * observeAreaWidth].pos;
                    //Animation* prevAnim = _tileAnimStates[x + 0 * observeAreaWidth].anim;
                    //prevAnim->reset();
                    for (int y = startY; incrY ? y < observeAreaWidth : y >= 0; incrY ? ++y : --y)
                    {
                        const int tileIndex = x + y * observeAreaWidth;
                        pk::vec3 currentPos = _tileAnimStates[tileIndex].pos;
                        //Animation* currentAnim = _tileAnimStates[tileIndex].anim;
                        //tempAnim->copyStateFrom(*currentAnim);
                        _tileAnimStates[tileIndex].pos = prevPos;
                        //_tileAnimStates[tileIndex].anim->copyStateFrom(*prevAnim);

                        prevPos = currentPos;
                        //prevAnim->copyStateFrom(*tempAnim);
                    }
                }
            }
        }

        //delete tempAnim;

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

        // Need to add little offset cuz using vertices as "tiles"
        float halfTileWidth = _tileVisualScale * 0.5f;
        float displacedWorldX = _worldX + halfTileWidth;
        float displacedWorldZ = _worldZ + halfTileWidth;
        int32_t tileX = (int32_t)std::floor(displacedWorldX / _tileVisualScale);
        int32_t tileY = (int32_t)std::floor(displacedWorldZ / _tileVisualScale);


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
                if (s_TEST_allowSend)
                {
                    if (elapsedSinceLastSend.count() >= 0.3f)
                    {
                        if (tileX != _observer.lastReceivedMapX || tileY != _observer.lastReceivedMapY)
                        {
                            Debug::log("___TEST___sending pos update");
                            _observer.requestedMapX = tileX;
                            _observer.requestedMapY = tileY;
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
                            s_TEST_skipUpdate = true;
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

    vec3 World::getMousePickCoords(const pk::mat4& projMat, const pk::mat4& viewMat) const
    {
        int mouseX = Application::get()->accessInputManager()->getMouseX();
        int mouseY = Application::get()->accessInputManager()->getMouseY();

        vec3 screenToWorldSpace = screen_to_world_space(mouseX, mouseY, projMat, viewMat);
        screenToWorldSpace.normalize();

        Transform* pCamTransform = (Transform*)_sceneRef.getComponent(_sceneRef.activeCamera, ComponentType::PK_TRANSFORM);

        mat4 camTMat = pCamTransform->getTransformationMatrix();
        vec3 startPos(camTMat[0 + 3 * 4], camTMat[1 + 3 * 4], camTMat[2 + 3 * 4]);

        const float maxPickingDist = 500.0f;
        const int maxPickRecursionCount = 500;


        return getMidpoint(startPos, screenToWorldSpace * maxPickingDist, maxPickRecursionCount);
    }

    vec3 World::getMidpoint(vec3 rayStartPos, vec3 ray, int recCount) const
    {
        vec3 halfRay = ray * 0.5f;
        vec3 midPoint = rayStartPos + halfRay;

        if (recCount <= 0)
        {
            return midPoint;
        }
        else
        {
            //float terrHeight = -terrain->getHeightAt(midPoint.x, midPoint.z);
            //float terrainHeight = getTileVisualHeightAt(midPoint.x, midPoint.z);
            float terrainHeight = getTerrainHeight(midPoint.x, midPoint.z);
            if (midPoint.y < terrainHeight)
            {
                return getMidpoint(rayStartPos, halfRay, recCount - 1);
            }
            else
            {
                return getMidpoint(midPoint, halfRay, recCount - 1);
            }
        }
    }

    void World::updateBlendmapData(PK_ubyte tileType, int x, int y)
    {
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
}
