
#include <algorithm>
#include <cstring>

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

		// TODO: Fetch "object info library" from server when logging in / connecting
		_objectInfo.push_back({"Empty", "", 0, nullptr, false});
        	_objectInfo.push_back({"Tree1", "A testing tree object", 0, _spriteTextures[0], false});
        	_objectInfo.push_back({"Movement Test", "For testing movement stuff", 1, _spriteTextures[1], true});
		
		// Create visual tiles at first as "blank" -> we configure these eventually, when we fetch world state from server
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

				VisualTile t(tileEntity, tileRenderable, effectRenderable, objectRenderable);

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
		//		std::make_pair(
		//			(PK_ubyte)TileStateTerrEffectFlags::TILE_STATE_terrEffectRain, 
		//			effectSpriteAnimator
		//		)
		//);
		//effectSpriteAnimator->enableLooping(true);
		//effectSpriteAnimator->play();

		// Create "tile progressions table"
		const size_t tileCount = observeAreaWidth * observeAreaWidth;
		_tileMovements.resize(tileCount);
		for (int i = 0; i < tileCount; ++i)
			_tileMovements[i] = vec2(0, 0);

		// Add OnMessageEvent to client which updates world state according to received server info
		Client* client = Client::get_instance();
		client->addOnMessageEvent(MESSAGE_TYPE__GetWorldState, new OnMessage_worldState(*this));
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
						if ((x == observeAreaWidth - 1 && i == 1) || (x == 0 && i == 0)) 
							continue;

						int heightIndex = i + j * 2;
						int sharedVertexIndex = (x + i) + (y + j) * observeAreaWidth;
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

				const float spriteWorldX = ((float)tileRenderable->worldX + _tileVisualScale * 0.5f) + _tileMovements[tileIndex].x;
				const float spriteWorldZ = ((float)tileRenderable->worldZ + _tileVisualScale * 0.5f) + _tileMovements[tileIndex].y;
				
				Sprite3DRenderable* tileEffectSprite = _tileData[tileIndex].second.renderable_effect;
				objects::VisualObject& visualObject = _tileData[tileIndex].second.visualObject;

				// * Currently no effects exists yet!
				tileEffectSprite->setActive(false);
				
				// Reset movements if no action, even in case we didn't have any object here
				if (!tileAction)
					_tileMovements[tileIndex] = vec2(0, 0);

				// Set tile object sprite
				if (tileObject)
				{
					const float tileVisualHeight = getTileVisualHeightAt(spriteWorldX, spriteWorldZ);
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
							tileVisualHeight, 
							spriteWorldZ
						);
						// Testing movement thing...
						const float objSpeed = 1.0f;
						if (tileAction == TileStateAction::TILE_STATE_actionMove)
							moveObjSprite(objDir, tileIndex, objSpeed);
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
				}
			}
		}
	}

	void VisualWorld::moveObjSprite(int dir, int tileIndex, float speed)
	{
		// *default val is North
		vec2 dirVec(0.0f, -1.0f);
		bool diag = false;
		switch (dir)
		{
			case TileStateDirection::TILE_STATE_dirN:
				break;
			case TileStateDirection::TILE_STATE_dirNE:
				dirVec = vec2(1.0f, -1.0f);
				diag = true;
				break;
			case TileStateDirection::TILE_STATE_dirE:
				dirVec = vec2(1.0f, 0.0f);
				break;
			case TileStateDirection::TILE_STATE_dirSE:
				dirVec = vec2(1.0f, 1.0f);
				diag = true;
				break;
			case TileStateDirection::TILE_STATE_dirS:
				dirVec = vec2(0.0, 1.0f);
				break;
			case TileStateDirection::TILE_STATE_dirSW:
				dirVec = vec2(-1.0f, 1.0f);
				diag = true;
				break;
			case TileStateDirection::TILE_STATE_dirW:
				dirVec = vec2(-1.0f, 0.0f);
				break;
			case TileStateDirection::TILE_STATE_dirNW:
				dirVec = vec2(-1.0f, -1.0f);
				diag = true;
				break;
			default:
				break;
		}
		
		dirVec.normalize();
		if (diag)
			dirVec = dirVec * 1.4f;

		float& visualObjX = _tileMovements[tileIndex].x;
		float& visualObjZ = _tileMovements[tileIndex].y;
		visualObjX += dirVec.x * speed * 2.0f * Timing::get_delta_time();
		visualObjZ += dirVec.y * speed * 2.0f * Timing::get_delta_time();
	}

	// Shifts "movements"-table, if moved camera, to make it look smooth
	void VisualWorld::shift(int32_t tileX, int32_t tileY)
	{
		const int observeAreaWidth = _observer.observeRadius * 2 + 1;

		if (tileX < _prevTileX)
		{
			// moving right (shift left)
			for (int y = 0; y < observeAreaWidth; ++y)
			{
				vec2 prev(0, 0);
				for (int x = 0; x < observeAreaWidth; ++x)
				{
					const int tileIndex = x + y * observeAreaWidth;
					vec2 current = _tileMovements[tileIndex];
					_tileMovements[tileIndex] = prev;
					prev = current;
				}
			}
		}
		if (tileX > _prevTileX)
		{
			// moving left (shift right)
			for (int y = 0; y < observeAreaWidth; ++y)
			{
				vec2 prev(0, 0);
				for (int x = observeAreaWidth - 1; x >= 0; --x)
				{
					const int tileIndex = x + y * observeAreaWidth;
					vec2 current = _tileMovements[tileIndex];
					_tileMovements[tileIndex] = prev;
					prev = current;
				}
			}
		}
		if (tileY < _prevTileY)
		{
			// moving forward(north) (shift back)
			for (int x = 0; x < observeAreaWidth; ++x)
			{
				vec2 prev(0, 0);
				for (int y = 0; y < observeAreaWidth; ++y)
				{
					const int tileIndex = x + y * observeAreaWidth;
					vec2 current = _tileMovements[tileIndex];
					_tileMovements[tileIndex] = prev;
					prev = current;
				}
			}
		}
		if (tileY > _prevTileY)
		{
			// moving backwards(south) (shift forward)
			for (int x = 0; x < observeAreaWidth; ++x)
			{
				vec2 prev(0, 0);
				for (int y = observeAreaWidth - 1; y >= 0 ; --y)
				{
					const int tileIndex = x + y * observeAreaWidth;
					vec2 current = _tileMovements[tileIndex];
					_tileMovements[tileIndex] = prev;
					prev = current;
				}
			}
		}
		
		// Save previous tile pos
		_prevTileX = tileX;
		_prevTileY = tileY;
	}

	void VisualWorld::update(float worldX, float worldZ)
	{
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
