#pragma once


#include "../../pk/core/Scene.h"

#include "../../pk/ecs/components/renderable/TerrainTileRenderable.h"
#include "../../pk/ecs/components/renderable/Sprite3DRenderable.h"
#include "../../pk/ecs/systems/animations/SpriteAnimations.h"

#include "../net/requests/Request.h"
#include "../../pk/Common.h"
#include "../../pk/utils/pkmath.h"

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace world
{

	struct WorldObserver
	{
		int observeRadius = 5;

		// Coordinates where we requested our latest "world state update" (not immediately received from server!)
		int requestedMapX = 0;
		int requestedMapY = 0;

		// Coordinates where we got the latest "world state update" successfully
		int lastReceivedMapX = 0;
		int lastReceivedMapY = 0;
	};


	// *NOTE! Visual tile doesnt own any of this mem. Its just collection of ptrs to elsewhere managed mem
	struct VisualTile
	{
		pk::TerrainTileRenderable*  renderable_tile		=	nullptr;
		pk::Sprite3DRenderable*		renderable_effect	=	nullptr;
		pk::Sprite3DRenderable*		renderable_thing	=	nullptr;

		VisualTile(pk::TerrainTileRenderable* tile, pk::Sprite3DRenderable* effect, pk::Sprite3DRenderable* thing)
		{
			renderable_tile		=	tile;
			renderable_effect	=	effect;
			renderable_thing	=	thing;
		}

		VisualTile(const VisualTile& other)
		{
			renderable_tile		=	other.renderable_tile;
			renderable_effect	=	other.renderable_effect;
			renderable_thing	=	other.renderable_thing;
		}
	};

	//	Visual representation of server's "world state" at currently observed coordinates
	class VisualWorld
	{
	private:

		class OnCompletion_fetchWorldState : public net::OnCompletionEvent
		{
		public:
			VisualWorld& visualWorldRef;
			OnCompletion_fetchWorldState(world::VisualWorld& visualWorld) : visualWorldRef(visualWorld) {}
			virtual void func(const PK_byte* data, size_t dataSize);
		};


		pk::Scene& _sceneRef;
		// Tile data acquired from the server. First = the actual tile data, second = just the "visual renderable"
		std::vector<std::pair<uint64_t, VisualTile>> _tileData;

		float _tileVisualScale = 2.0f;

		WorldObserver _observer;

		
		float _maxUpdateCooldown = 0.5f;
		float _updateCooldown = 0.0f;

		PK_byte* _pBlendmapData = nullptr;
		int _blendmapWidth = 0;
		// Just temp here -> clean up later..


		std::unordered_map<PK_ubyte, pk::SpriteAnimator*> _tileEffectAnimMapping;

	public:

		VisualWorld(pk::Scene& scene, int observeRadius);
		~VisualWorld();

		void updateObservedArea(const uint64_t* mapState);

		void update(float worldX, float worldZ);
		inline std::vector<std::pair<uint64_t, VisualTile>>& getObservedTiles() { return _tileData; }

		float getTileVisualHeightAt(float worldX, float worldZ) const;

		pk::vec3 getMousePickCoords(const pk::mat4& projMat, const pk::mat4& viewMat) const;

	private:

		pk::vec3 getMidpoint(pk::vec3 rayStartPos, pk::vec3 ray, int recCount) const;

		void updateBlendmapData(PK_ubyte tileType, int x, int y);
	};

}