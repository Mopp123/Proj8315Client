#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <chrono>

#include "../../PortablePesukarhu/ppk.h"
#include "../net/Client.h"
#include "Objects.h"
#include "Tile.h"
#include "../../Proj8315Common/src/Faction.h"


namespace world
{
    struct WorldObserver
    {
        int32_t observeRadius = 5;

        // Coordinates where we requested our latest
        // "world state update" (not immediately received from server!)
        int32_t requestedMapX = 0;
        int32_t requestedMapY = 0;

        // Coordinates where we got the latest "world state update" successfully
        int32_t lastReceivedMapX = 0;
        int32_t lastReceivedMapY = 0;
    };


    // NOTE: Visual tile doesnt own any of the mem of these ptrs.
    // Its just collection of ptrs to elsewhere managed mem
    struct VisualTile
    {
        uint32_t entity = 0;
        pk::TerrainTileRenderable* renderable_tile = nullptr;
        pk::Sprite3DRenderable* renderable_effect = nullptr;
        objects::VisualObject visualObject;

        VisualTile(
            pk::Scene& scene,
            World& worldRef,
            uint32_t owningEntity,
            pk::TerrainTileRenderable* tile,
            pk::Sprite3DRenderable* effect,
            pk::Sprite3DRenderable* object
        ) :
            entity(owningEntity),
            visualObject(scene, worldRef, object)
        {
            renderable_tile = tile;
            renderable_effect = effect;
        }

        VisualTile(const VisualTile& other):
            entity(other.entity),
            visualObject(other.visualObject)
        {
            renderable_tile = other.renderable_tile;
            renderable_effect = other.renderable_effect;
        }
    };


    // Represents server's "world state"
    // Makes world state to be rendered at observed coordinates
    class World
    {
    private:
        class OnMessageWorldState : public net::OnMessageEvent
        {
        public:
            World& visualWorldRef;

            OnMessageWorldState(World& visualWorld) :
                visualWorldRef(visualWorld)
            {}
            ~OnMessageWorldState() {}

            virtual void onMessage(const PK_byte* data, size_t dataSize);
        };

        class TileAnimation
        {
        public:
            pk::vec3 pos = pk::vec3(0, 0, 0);
            pk::Animation* anim = nullptr;

            TileAnimation()
            {}

            TileAnimation(pk::vec3 position, pk::Animation* animation) :
                pos(position)
            {
                anim = animation;
            }

            void reset()
            {
                pos = pk::vec3(0, 0, 0);
                anim->reset();
            }

            void set(const TileAnimation& other)
            {
                pos = other.pos;
                if (anim)
                    anim->copyFrom(*other.anim);
            }
        };

        pk::Scene& _sceneRef;

        // Tile data acquired from the server.
        //  First = the actual tile data
        //  second = just the "visual renderable"
        std::vector<std::pair<uint64_t, VisualTile>> _tileData;
        // used for animating movement and sprites of a tile
        std::vector<TileAnimation> _tileAnimStates; // *Previously "tileMovements"

        float _tileVisualScale = 2.0f;

        float _worldX = 0.0f;
        float _worldZ = 0.0f;
        int _prevTileX = 0;
        int _prevTileY = 0;
        WorldObserver _observer;

        PK_byte* _pBlendmapData = nullptr;
        int _blendmapWidth = 0;

        pk::Transform* _pCamTransform = nullptr;
        // Facing direction of the camera. In form of TileStateDirection::north, etc..
        int _cameraDirection = 0;

        std::unordered_map<std::string, gamecommon::Faction> _factions;

    public:
        World(pk::Scene& scene, pk::Transform* pCamTransform, int observeRadius);
        ~World();

        void updateObservedArea(const uint64_t* mapState);
        void update(float worldX, float worldZ);

        // Adds faction if doesn't exist yet
        void addFaction(const gamecommon::Faction& faction);
        // Updates faction if already exists
        void updateFaction(const gamecommon::Faction& faction);
        // Updates faction if exists or adds a new faction if doesn't exist
        void updateFactionList(const std::string& factionName, const gamecommon::Faction& faction);
        gamecommon::Faction getFaction(const std::string& factionName) const;
        bool factionExists(const std::string& factionName) const;

        float getTileVisualHeightAt(float worldX, float worldZ) const;

        pk::vec3 getMousePickCoords(const pk::mat4& projMat, const pk::mat4& viewMat) const;

    private:
        void updateSprites();
        // Shifts "movements"-table, if moved camera, to make it look smooth
        void shift(int32_t tileX, int32_t tileY);

        pk::vec3 getMidpoint(pk::vec3 rayStartPos, pk::vec3 ray, int recCount) const;

        void updateBlendmapData(PK_ubyte tileType, int x, int y);
    };
}

