
#include <algorithm>
#include <cstring>
#include <string>

#include "Object.h"
#include "World.h"
#include "Tile.h"
#include "../net/NetCommon.h"

using namespace pk;
using namespace pk::web;

using namespace net;

namespace world
{
    VisualWorld::OnMessage_worldState::OnMessage_worldState(VisualWorld& visualWorld) :
        visualWorldRef(visualWorld)
    {}

    VisualWorld::OnMessage_worldState::~OnMessage_worldState()
    {}

    void VisualWorld::OnMessage_worldState::onMessage(const PK_byte* data, size_t dataSize)
    {
        WorldObserver& observerRef = visualWorldRef._observer;
        const int dataWidth = (observerRef.observeRadius * 2) + 1;
        const size_t expectedDataSize = (dataWidth * dataWidth) * sizeof(uint64_t);
        if (dataSize >= expectedDataSize)
        {
            const uint64_t* dataBuf = (const uint64_t*)data;

            visualWorldRef.updateObservedArea(dataBuf);
            
            observerRef.lastReceivedMapX = visualWorldRef._observer.requestedMapX;
            observerRef.lastReceivedMapY = visualWorldRef._observer.requestedMapY;

            // TODO: Delete below, deprecated after moving away from "requests"!!!
            Client* client = Client::get_instance();
            size_t bufSize = sizeof(int) * 4;
            PK_byte* pSendBuf = new PK_byte[bufSize];
            memset(pSendBuf, 0, bufSize);
            const int32_t messageType = MESSAGE_TYPE__UpdateObserverProperties;
            
            memcpy(pSendBuf, &messageType, sizeof(int32_t));
            memcpy(pSendBuf + sizeof(int32_t), (const void*)&observerRef.requestedMapX, sizeof(int32_t));
            memcpy(pSendBuf + sizeof(int32_t) * 2, (const void*)&observerRef.requestedMapY, sizeof(int32_t));
            memcpy(pSendBuf + sizeof(int32_t) * 3, (const void*)&observerRef.observeRadius, sizeof(int32_t));
            
            client->send(pSendBuf, bufSize);

            delete[] pSendBuf;
        }
    }


    VisualWorld::OnMessage_objInfoLib::OnMessage_objInfoLib(VisualWorld& visualWorld) :
        visualWorldRef(visualWorld)
    {}

    VisualWorld::OnMessage_objInfoLib::~OnMessage_objInfoLib()
    {}

    void VisualWorld::OnMessage_objInfoLib::onMessage(const PK_byte* data, size_t dataSize)
    {
        if (visualWorldRef._worldInitialized)
            return;
        visualWorldRef._worldInitialized = true;

        std::vector<objects::ObjectInfo>& objInfoLibRef = visualWorldRef._objectInfo;
        objInfoLibRef.clear();
        const size_t objCount = dataSize / objects::get_netw_objinfo_size();
        int currentDataPos = 0;
        for (size_t i = 0; i < objCount; ++i)
        {
            PK_byte name[OBJECT_DATA_STRLEN_NAME];
            memcpy(name, data + currentDataPos, OBJECT_DATA_STRLEN_NAME);
            std::string name_str(name, OBJECT_DATA_STRLEN_NAME);

            PK_byte description[OBJECT_DATA_STRLEN_DESCRIPTION];
            memcpy(description, data + currentDataPos + OBJECT_DATA_STRLEN_NAME, OBJECT_DATA_STRLEN_DESCRIPTION);
            std::string description_str(description, OBJECT_DATA_STRLEN_DESCRIPTION);
            
            std::vector<std::string> actions;
            int currentPtr = OBJECT_DATA_STRLEN_NAME + OBJECT_DATA_STRLEN_DESCRIPTION;
            for (int j = 0; j < TILE_STATE_MAX_action + 1; ++j)
            {
                PK_byte action[OBJECT_DATA_STRLEN_ACTION_NAME];
                memcpy(action, data + currentDataPos + currentPtr, OBJECT_DATA_STRLEN_ACTION_NAME);
                std::string action_str(action, OBJECT_DATA_STRLEN_ACTION_NAME);
                actions.push_back(action_str);
                currentPtr += OBJECT_DATA_STRLEN_ACTION_NAME;
            }
            // obj stats..
            PK_ubyte speed = 0;
            memcpy(&speed, data + currentDataPos + currentPtr, 1);
            objects::ObjectInfo objInfo(name_str, description_str, actions, speed);
            // TODO: Make below somehow more less dumb..
            // Set these objects' visual properties
            // *use 'i' since index of that list is equal to the object's type id
            switch (i)
            {
                case 1:
                    objInfo.pTexture = visualWorldRef._spriteTextures[0];
                    break;
                case 2:
                    objInfo.rotateableSprite = true;
                    objInfo.pTexture = visualWorldRef._spriteTextures[1];
                    break;
                case 3:
                    objInfo.pTexture = visualWorldRef._spriteTextures[2];
                    break;
                default:
                    break;
            }
            objInfoLibRef.push_back(objInfo);
            currentDataPos += currentPtr + 1; // +1 since the one byte stat
        }
    }


    VisualWorld::VisualWorld(pk::Scene& scene, pk::Transform* pCamTransform, int observeRadius) :
        _sceneRef(scene),
        _pCamTransform(pCamTransform)
    {
        _observer.observeRadius = observeRadius;

        // Load all sprite textures
        TextureSampler spriteTextureSampler =
        {
            TextureSamplerFilterMode::PK_SAMPLER_FILTER_MODE_LINEAR,
            TextureSamplerAddressMode::PK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            2
        };
        _spriteTextures.push_back(new WebTexture("assets/environment.png", spriteTextureSampler, 8));
        _spriteTextures.push_back(new WebTexture("assets/MovementTest.png", spriteTextureSampler, 8));
        _spriteTextures.push_back(new WebTexture("assets/landings.png", spriteTextureSampler, 4));



        // TODO: Fetch "object info library" from server when logging in / connecting
        //_objectInfo.push_back({"Empty", "", 0, nullptr, false});
            //_objectInfo.push_back({"Tree1", "A testing tree object", 0, _spriteTextures[0], false});
            //_objectInfo.push_back({"Movement Test", "For testing movement stuff", 1, _spriteTextures[1], true});
        
        // Create visual tiles at first as "blank" 
        //  -> we configure these eventually, when we fetch world state from server
        const int observeAreaWidth = _observer.observeRadius * 2 + 1;
        for (int y = 0; y < observeAreaWidth; ++y)
        {
            for (int x = 0; x < observeAreaWidth; ++x)
            {
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
            }
        }

        // Create blendmap
        int blendmapChannels = 4;
        _blendmapWidth = get_next_pow2(observeAreaWidth);
        size_t blendmapDataSize = (_blendmapWidth * _blendmapWidth) * blendmapChannels;
        _pBlendmapData = new PK_byte[blendmapDataSize];
        memset(_pBlendmapData, 0, blendmapDataSize);
        WebTexture* blendmapTexture = new WebTexture(
            _pBlendmapData, _blendmapWidth, _blendmapWidth, blendmapChannels,
            {
                TextureSamplerFilterMode::PK_SAMPLER_FILTER_MODE_LINEAR,
                TextureSamplerAddressMode::PK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                2
            }
        );
        TerrainTileRenderable::s_blendmapTexture = blendmapTexture;
        TerrainTileRenderable::s_gridWidth = _blendmapWidth;

        // init anim mappings
        //SpriteAnimator* effectSpriteAnimator = new SpriteAnimator({ {0,0},{1,0},{2,0},{1,0} }, 0.25f);
        //_sceneRef.addSystem(effectSpriteAnimator);
        //_tileEffectAnimMapping.insert(
        //      std::make_pair(
        //          (PK_ubyte)TileStateTerrEffectFlags::TILE_STATE_terrEffectRain, 
        //          effectSpriteAnimator
        //      )
        //);
        //effectSpriteAnimator->enableLooping(true);
        //effectSpriteAnimator->play();

        // Create "tile progressions/movements/anims table"
        const size_t tileCount = observeAreaWidth * observeAreaWidth;
        _tileAnimStates.resize(tileCount);
        for (int i = 0; i < tileCount; ++i)
        {
            Animation* anim = new Animation({ 0 }, 4.0f);
            anim->enableLooping(true);
            anim->play();
            TileAnimation tileAnim({ 0, 0, 0 }, anim);
            _tileAnimStates[i] = tileAnim;
            _tileAnimStates[i].anim = anim;
            scene.addSystem((pk::System*)anim);
        }

        // Add OnMessageEvents
        Client* client = Client::get_instance();
        // to update world state
        client->addOnMessageEvent(MESSAGE_TYPE__GetWorldState, new OnMessage_worldState(*this));
        // to update obj info lib
        client->addOnMessageEvent(MESSAGE_TYPE__GetObjInfoLib, new OnMessage_objInfoLib(*this));
    }

    VisualWorld::~VisualWorld()
    {
        delete[] _pBlendmapData;

        for (WebTexture* texture : _spriteTextures)
            delete texture;

        _spriteTextures.clear();
    }

    // ..quite shit and inefficient
    void VisualWorld::updateObservedArea(const uint64_t* mapState)
    {
        const int observeAreaWidth = _observer.observeRadius * 2 + 1;

        // this is used to group together all tiles' vertices which share the same pos
        std::unordered_map<int, float> sharedVertexHeights;
        
        // Update shared heights mapping
        for (int y = 0; y < observeAreaWidth; ++y)
        {
            for (int x = 0; x < observeAreaWidth; ++x)
            {
                const int tileIndex = x + y * observeAreaWidth;
                uint64_t tileState = mapState[tileIndex];
                _tileData[tileIndex].first = tileState;

                TerrainTileRenderable* tileRenderable = _tileData[tileIndex].second.renderable_tile;
                const int32_t tileWorldX = ((float)_observer.requestedMapX + (float)x) * _tileVisualScale;
                const int32_t tileWorldZ = ((float)_observer.requestedMapY + (float)y) * _tileVisualScale;
                tileRenderable->worldX = tileWorldX;
                tileRenderable->worldZ = tileWorldZ;

                float height = (float)(get_tile_terrelevation(tileState));
                const float max = 15.0f;
                if (height - max >= 0.0f)
                    height -= max;
                //height *= 2.0f;

                for (int j = 0; j < 2; ++j)
                {
                    for (int i = 0; i < 2; ++i)
                    {
                        int sharedHeightIndex = (x + i) + (y + j) * observeAreaWidth;
                        sharedVertexHeights[sharedHeightIndex] = std::max(sharedVertexHeights[sharedHeightIndex], height);
                    }
                }

                // Alter texturing depending on terrain type
                PK_ubyte tileType = get_tile_terrtype(tileState);
                updateBlendmapData(tileType, x, y);

                // Assign anim frames
                objects::VisualObject& visualObj = _tileData[tileIndex].second.visualObject;
                const PK_ubyte tileObject = get_tile_thingid(tileState);
                const PK_ubyte tileAction = get_tile_action(tileState);
                visualObj.assignAnimFrames(tileObject, tileAction, _tileAnimStates[tileIndex].anim);
            }
        }

        // Tiles' vertices' heights' combining/smoothing according to "sharedVertexHeights"-mapping
        for (int y = 0; y < observeAreaWidth; ++y)
        {
            for (int x = 0; x < observeAreaWidth; ++x)
            {
                int tileIndex = x + y * observeAreaWidth;
                TerrainTileRenderable* tileRenderable = _tileData[tileIndex].second.renderable_tile;

                for (int j = 0; j < 2; ++j)
                {
                    for (int i = 0; i < 2; ++i)
                    {
                        // This looks fucking disqusting, but for now it prevents "heightbleedingbugthing"
						int addX = 0;
						if (x == 0 && i == 0)
							addX = 1;
						if (x == observeAreaWidth - 1 && i == 1)
							addX = -1;

						int heightIndex = i + j * 2;
						int sharedVertexIndex = (x + i + addX) + (y + j) * observeAreaWidth;
						tileRenderable->vertexHeights[heightIndex] = sharedVertexHeights[sharedVertexIndex];
                    }
                }

            }
        }

        TerrainTileRenderable::s_blendmapTexture->update(_pBlendmapData);
    }

    // Updates tile sprites
    // * Has to be done after updating terrain heights and very frequently to not look funny..
    void VisualWorld::updateSprites()
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
                    if (tileObject < _objectInfo.size())
                    {
                        visualObject.show(
                            tileObject, 
                            tileAction, 
                            objDir,
                            _cameraDirection,
                            _objectInfo[tileObject],
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

    // Shifts "movements"-table, if moved camera, to make it look smooth
    void VisualWorld::shift(int32_t tileX, int32_t tileY)
    {
        const int observeAreaWidth = _observer.observeRadius * 2 + 1;

        Animation* tempAnim = new Animation({ 0 }, 1.0f);
        // moving right(default) (shift left)
        int startX = tileX > _prevTileX ? observeAreaWidth - 1 : 0;
        int startY = tileY > _prevTileY ? observeAreaWidth - 1 : 0;
        bool incrX = !(tileX > _prevTileX);
        bool incrY = !(tileY > _prevTileY);

		// Horizontal shifting
		if (tileX != _prevTileX)
		{
        	for (int y = 0; y < observeAreaWidth; ++y)
        	{
        	    pk::vec3 prevPos = _tileAnimStates[0 + y * observeAreaWidth].pos;
        	    Animation* prevAnim = _tileAnimStates[0 + y * observeAreaWidth].anim;
        	    prevAnim->reset();
        	    for (int x = startX; incrX ? x < observeAreaWidth : x >= 0; incrX ? ++x : --x)
        	    {
        	        const int tileIndex = x + y * observeAreaWidth;
        	        pk::vec3 currentPos = _tileAnimStates[tileIndex].pos;
        	        Animation* currentAnim = _tileAnimStates[tileIndex].anim;
        	        tempAnim->copyFrom(*currentAnim);
        	        _tileAnimStates[tileIndex].pos = prevPos;
        	        _tileAnimStates[tileIndex].anim->copyFrom(*prevAnim);

        	        prevPos = currentPos;
        	        prevAnim->copyFrom(*tempAnim);
        	    }
        	}
		}
		// Vertical shifting
		if (tileY != _prevTileY)
		{
        	for (int x = 0; x < observeAreaWidth; ++x)
        	{
        	    pk::vec3 prevPos = _tileAnimStates[x + 0 * observeAreaWidth].pos;
        	    Animation* prevAnim = _tileAnimStates[x + 0 * observeAreaWidth].anim;
        	    prevAnim->reset();
        	    for (int y = startY; incrY ? y < observeAreaWidth : y >= 0; incrY ? ++y : --y)
        	    {
        	        const int tileIndex = x + y * observeAreaWidth;
        	        pk::vec3 currentPos = _tileAnimStates[tileIndex].pos;
        	        Animation* currentAnim = _tileAnimStates[tileIndex].anim;
        	        tempAnim->copyFrom(*currentAnim);
        	        _tileAnimStates[tileIndex].pos = prevPos;
        	        _tileAnimStates[tileIndex].anim->copyFrom(*prevAnim);

        	        prevPos = currentPos;
        	        prevAnim->copyFrom(*tempAnim);
        	    }
        	}
		}

        delete tempAnim;
        
        // Save previous tile pos
        _prevTileX = tileX;
        _prevTileY = tileY;
    }

    void VisualWorld::update(float worldX, float worldZ)
    {
        // Make sure the world is ready to be updated
        // *quite a quick dirty fix -> will lead to fuckups and weird crashes later on..
        if (!_worldInitialized)
        {
            // Fetch obj. info lib from server
            const int32_t msgTypeFetchObjInfo = MESSAGE_TYPE__GetObjInfoLib;
            PK_byte* sendBuf = new PK_byte[sizeof(int32_t)];
            memcpy(sendBuf, &msgTypeFetchObjInfo, sizeof(int32_t));
            Client::get_instance()->send(sendBuf, sizeof(int32_t));
            delete[] sendBuf;
            return;
        }

        _worldX = worldX;
        _worldZ = worldZ;

        // Calc the "map pos" according to "visual float pos"(this should be camera's pivot point, if rts style camera)
        int32_t tileX = (int32_t)std::floor((_worldX - (float)_observer.observeRadius * _tileVisualScale) / _tileVisualScale);
        int32_t tileY = (int32_t)std::floor((_worldZ - (float)_observer.observeRadius * _tileVisualScale) / _tileVisualScale);
        _observer.requestedMapX = tileX;
        _observer.requestedMapY = tileY;

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

        updateSprites();
        shift(tileX, tileY);
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

    float VisualWorld::getTileVisualHeightAt(float worldX, float worldZ) const
    {
        // Calc the "map pos" according to "visual float pos"

        const int gridWidth = (_observer.observeRadius * 2) + 1;
        
        int worldMapX = (int)std::floor(worldX / _tileVisualScale);
        int worldMapY = (int)std::floor(worldZ / _tileVisualScale);

        int mapX = worldMapX - _observer.lastReceivedMapX;
        int mapY = worldMapY - _observer.lastReceivedMapY;

        
        int tileIndex = mapX + mapY * gridWidth;
        if(tileIndex >= 0 && tileIndex < _tileData.size())
        {
            TerrainTileRenderable* tileRenderable = _tileData[tileIndex].second.renderable_tile;

            // Coordinates in relation to the current tile, in range 0 to 1
            float tileSpaceX = std::fmod(worldX, _tileVisualScale) / _tileVisualScale;
            float tileSpaceZ = std::fmod(worldZ, _tileVisualScale) / _tileVisualScale;

            

            const float height_tl = tileRenderable->vertexHeights[0];
            const float height_tr = tileRenderable->vertexHeights[1];
            const float height_bl = tileRenderable->vertexHeights[2];
            const float height_br = tileRenderable->vertexHeights[3];


            const int verticesPerRow = 2;

            // Check which triangle of the tile we are standing on..
            if (tileSpaceX <= tileSpaceZ) {
                return get_triangle_height_barycentric(
                    vec3(0, height_tl, 0),
                    vec3(0, height_bl, 1),
                    vec3(1, height_br, 1),
                    vec2(tileSpaceX, tileSpaceZ));
            }
            else {
                return get_triangle_height_barycentric(
                    vec3(0, height_tl, 0),
                    vec3(1, height_br, 1),
                    vec3(1, height_tr, 0),
                    vec2(tileSpaceX, tileSpaceZ));
            }
        }
        else
        {
            return 0.0f;
        }
    }



    vec3 VisualWorld::getMousePickCoords(const pk::mat4& projMat, const pk::mat4& viewMat) const
    {
        int mouseX = Application::get()->accessInputManager()->getMouseX();
        int mouseY = Application::get()->accessInputManager()->getMouseY();

        vec3 screenToWorldSpace = screen_to_world_space(mouseX, mouseY, projMat, viewMat);
        screenToWorldSpace.normalize();
        
        Transform* pCamTransform = (Transform*)_sceneRef.getComponent(_sceneRef.activeCamera->getEntity(), ComponentType::PK_TRANSFORM);
        
        mat4 camTMat = pCamTransform->getTransformationMatrix();
        vec3 startPos(camTMat[0 + 3 * 4], camTMat[1 + 3 * 4], camTMat[2 + 3 * 4]);

        const float maxPickingDist = 500.0f;
        const int maxPickRecursionCount = 500;


        return getMidpoint(startPos, screenToWorldSpace * maxPickingDist, maxPickRecursionCount);
    }

    vec3 VisualWorld::getMidpoint(vec3 rayStartPos, vec3 ray, int recCount) const
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
            float terrainHeight = getTileVisualHeightAt(midPoint.x, midPoint.z);
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


    void VisualWorld::updateBlendmapData(PK_ubyte tileType, int x, int y)
    {
        const int r = tileType == 1 ? 255 : 0;
        const int g = tileType == 2 ? 255 : 0;
        const int b = tileType == 3 ? 255 : 0;
        const int a = tileType == 4 ? 255 : 0;

        _pBlendmapData[(x + y * _blendmapWidth) * 4] = r;
        _pBlendmapData[(x + y * _blendmapWidth) * 4 + 1] = g;
        _pBlendmapData[(x + y * _blendmapWidth) * 4 + 2] = b;
        _pBlendmapData[(x + y * _blendmapWidth) * 4 + 3] = a;
    }
}
