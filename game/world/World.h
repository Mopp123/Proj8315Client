#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

#include "../../PortablePesukarhu/ppk.h"
#include "../net/Client.h"

namespace world
{
	struct WorldObserver
	{
		int32_t observeRadius = 5;

		// Coordinates where we requested our latest "world state update" (not immediately received from server!)
		int32_t requestedMapX = 0;
		int32_t requestedMapY = 0;

		// Coordinates where we got the latest "world state update" successfully
		int32_t lastReceivedMapX = 0;
		int32_t lastReceivedMapY = 0;
	};


	// *NOTE! Visual tile doesnt own any of this mem. Its just collection of ptrs to elsewhere managed mem
	struct VisualTile
	{
		uint32_t entity = 0;
		pk::TerrainTileRenderable* renderable_tile = nullptr;
		pk::Sprite3DRenderable*	renderable_effect = nullptr;
		pk::Sprite3DRenderable*	renderable_object = nullptr;

		VisualTile(uint32_t owningEntity, pk::TerrainTileRenderable* tile, pk::Sprite3DRenderable* effect, pk::Sprite3DRenderable* object) :
			entity(owningEntity)
		{
			renderable_tile	= tile;
			renderable_effect = effect;
			renderable_object = object;
		}

		VisualTile(const VisualTile& other):
			entity(other.entity)
		{
			renderable_tile	= other.renderable_tile;
			renderable_effect = other.renderable_effect;
			renderable_object = other.renderable_object;
		}
	};


	// Visual representation of server's "world state" at currently observed coordinates
	class VisualWorld
	{
	private:
		/*
		class OnCompletion_fetchWorldState : public net::OnCompletionEvent
		{
		public:
			VisualWorld& visualWorldRef;
			OnCompletion_fetchWorldState(world::VisualWorld& visualWorld) : visualWorldRef(visualWorld) {}
			virtual void func(const PK_byte* data, size_t dataSize);
		};
		*/
		class OnMessage_worldState : public net::OnMessageEvent
		{
		public:
			VisualWorld& visualWorldRef;
			
			OnMessage_worldState(VisualWorld& visualWorld);
			~OnMessage_worldState();
			virtual void onMessage(const PK_byte* data, size_t dataSize);
		};
		OnMessage_worldState* onMessage_worldState = nullptr;

		pk::Scene& _sceneRef;
		// Tile data acquired from the server. First = the actual tile data, second = just the "visual renderable"
		std::vector<std::pair<uint64_t, VisualTile>> _tileData;
		std::vector<pk::vec2> _tileMovements;

		float _tileVisualScale = 2.0f;
		
		float _worldX = 0.0f;
		float _worldZ = 0.0f;
		int _prevTileX = 0;
		int _prevTileY = 0;
		WorldObserver _observer;

		float _maxUpdateCooldown = 0.01f;
		float _updateCooldown = 0.0f;

		PK_byte* _pBlendmapData = nullptr;
		int _blendmapWidth = 0;
		
		// Just temp here -> clean up later..
		std::unordered_map<PK_ubyte, pk::SpriteAnimator*> _tileEffectAnimMapping;

	public:
		VisualWorld(pk::Scene& scene, int observeRadius, pk::Texture* effectsTexture, pk::Texture* objectsTexture);
		~VisualWorld();

		void updateObservedArea(const uint64_t* mapState);

		void update(float worldX, float worldZ);
		inline std::vector<std::pair<uint64_t, VisualTile>>& getObservedTiles() { return _tileData; }

		float getTileVisualHeightAt(float worldX, float worldZ) const;

		pk::vec3 getMousePickCoords(const pk::mat4& projMat, const pk::mat4& viewMat) const;

	private:
		void updateSprites();
		void moveObjSprite(std::pair<uint64_t, VisualTile>& tile, int tileIndex, float speed);
		// Shifts "movements"-table, if moved camera, to make it look smooth
		void shift(int32_t tileX, int32_t tileY);
		
		pk::vec3 getMidpoint(pk::vec3 rayStartPos, pk::vec3 ray, int recCount) const;

		void updateBlendmapData(PK_ubyte tileType, int x, int y);
	};
}

