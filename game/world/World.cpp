
#include "World.h"
#include "Tile.h"
#include "../../pk/graphics/platform/web/WebTexture.h"
#include "../net/Client.h"
#include "../net/requests/Commands.h"
#include "../net/NetCommon.h"

#include "../../pk/core/Application.h"
#include "../../pk/utils/Algorithms.h"
#include "../../pk/core/Timing.h"

#include <algorithm>

using namespace pk;
using namespace pk::web;

using namespace net;
using namespace net::web;

namespace world
{



	void VisualWorld::OnCompletion_fetchWorldState::func(const PK_byte* data, size_t dataSize)
	{
		
		WorldObserver& observerRef = visualWorldRef._observer;

		const int dataWidth = (observerRef.observeRadius * 2) + 1;
		const size_t expectedDataSize = (dataWidth * dataWidth) * sizeof(uint64_t);

		// Attempt to print fetched area
		if (dataSize >= expectedDataSize)
		{
			const uint64_t* dataBuf = (const uint64_t*)data;
			visualWorldRef.updateObservedArea(dataBuf);

			observerRef.lastReceivedMapX = visualWorldRef._observer.requestedMapX;
			observerRef.lastReceivedMapY = visualWorldRef._observer.requestedMapY;
		}
	}


	VisualWorld::VisualWorld(pk::Scene& scene, int observeRadius) :
		_sceneRef(scene)
	{
		_observer.observeRadius = observeRadius;


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

				Sprite3DRenderable* effectRenderable = new Sprite3DRenderable({ x * _tileVisualScale, 0, y * _tileVisualScale }, { _tileVisualScale, _tileVisualScale });

				_sceneRef.addComponent(tileEntity, tileRenderable);
				_sceneRef.addComponent(tileEntity, effectRenderable);

				VisualTile t{ tileRenderable, effectRenderable, nullptr };

				_tileData.push_back(std::make_pair(0, t));
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
		SpriteAnimator* effectSpriteAnimator = new SpriteAnimator({ {0,0},{1,0},{2,0},{1,0} }, 0.25f);
		_sceneRef.addSystem(effectSpriteAnimator);
		_tileEffectAnimMapping.insert(std::make_pair((PK_ubyte)TileStateTerrEffectFlags::TILE_STATE_terrEffectRain, effectSpriteAnimator));
		
		effectSpriteAnimator->enableLooping(true);
		effectSpriteAnimator->play();
	}

	VisualWorld::~VisualWorld()
	{
		delete[] _pBlendmapData;
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
				tileRenderable->worldX = (_observer.requestedMapX + x) * _tileVisualScale;
				tileRenderable->worldZ = (_observer.requestedMapY + y) * _tileVisualScale;

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

				Debug::log("Received tiletype was: " + std::to_string(tileType));

				// Alter effect sprite, if there are effects on the tile
				// JUST TESTING ATM!!!
				PK_ubyte tileEffect = get_tile_terreffect(tileState);
				Sprite3DRenderable* tileEffectSprite = _tileData[tileIndex].second.renderable_effect;				
				
				if (tileEffect & (PK_ubyte)TileStateTerrEffectFlags::TILE_STATE_terrEffectRain)
				{
					Debug::log("Received tile was raining!");
					tileEffectSprite->position = vec3(tileRenderable->worldX + _tileVisualScale * 0.5f, tileRenderable->getAverageHeight() + 2.0f, tileRenderable->worldZ + _tileVisualScale * 0.5f);
					tileEffectSprite->setActive(true);
					tileEffectSprite->textureOffset = _tileEffectAnimMapping[tileEffect]->getCurrentTexOffset();
				}
				else
				{
					tileEffectSprite->setActive(false);
				}
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
						if ((x == observeAreaWidth - 1 && i == 1) || (x == 0 && i == 0)) // This looks fucking disqusting, but for now it prevents "heightbleedingbugthing"
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


	void VisualWorld::update(float worldX, float worldZ)
	{
		// Calc the "map pos" according to "visual float pos"(this should be camera's pivot point, if rts style camera)
		_observer.requestedMapX = (int)std::floor((worldX - (float)_observer.observeRadius * _tileVisualScale) / _tileVisualScale);
		_observer.requestedMapY = (int)std::floor((worldZ - (float)_observer.observeRadius * _tileVisualScale) / _tileVisualScale);

		//Debug::log("observer pos: " + std::to_string(_observer.requestedMapX) + ", " + std::to_string(_observer.requestedMapY));

		if (_updateCooldown <= 0.0f)
		{
			send_command(Client::get_instance()->getUserID(), CMD_FetchWorldState, _observer.requestedMapX, _observer.requestedMapY, _observer.observeRadius, new OnCompletion_fetchWorldState(*this));
			_updateCooldown = _maxUpdateCooldown;
		}
		else
		{
			_updateCooldown -= 1.0f * Timing::get_delta_time();
		}
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