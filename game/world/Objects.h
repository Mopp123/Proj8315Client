#pragma once

#include <vector>
#include <unordered_map>

#include "../../PortablePesukarhu/ppk.h"
#include "Tile.h"
#include "../../Proj8315Common/src/Object.h"


#define OBJECT_DATA_STRLEN_NAME 32
#define OBJECT_DATA_STRLEN_DESCRIPTION 32
#define OBJECT_DATA_STRLEN_ACTION_NAME 16

#define OBJECT_DATA_COUNT_STATS 1
#define OBJECT_DATA_COUNT_TOTAL ((1 + 1) + (TILE_STATE_MAX_action + 1) + OBJECT_DATA_COUNT_STATS)


namespace world
{
    class World;

    namespace objects
    {
        // NEW
        class VisualObjectInfo
        {
        public:
            pk::web::WebTexture* pTexture = nullptr; // *NOTE! ObjectInfo doesn't own its' texture
            // How to crop texture to display portrait in ui
            class TexturePortraitCropping
            {
            public:
                // How many individual images in sprite sheet
                float rowCount = 8.0f;
                pk::vec2 pos;
                pk::vec2 scale;

                TexturePortraitCropping() :
                    TexturePortraitCropping (8.0f, { 1.0f, 3.0f }, 64.0f)
                {}
                TexturePortraitCropping(float rows, pk::vec2 cropBegin, float cropScale)
                {
                    rowCount = rows;
                    pos = pk::vec2(cropBegin.x / rowCount, cropBegin.y / rowCount);
                    scale = pk::vec2(cropScale / (rowCount * cropScale), cropScale / (rowCount * cropScale));
                }
            };

            TexturePortraitCropping portraitCropping;
            bool rotateableSprite = false;

            static TexturePortraitCropping s_defaultPortraitCropping;

            VisualObjectInfo() {}
            VisualObjectInfo(const VisualObjectInfo& other):
                pTexture(other.pTexture),
                portraitCropping(other.portraitCropping),
                rotateableSprite(other.rotateableSprite)
            {
            }
        };


        // NOTE: Visual object doesnt own any of mem of these ptrs.
        // Its just collection of ptrs to elsewhere managed mem
        // (atm like like World's "VisualTile")
        class VisualObject
        {
        private:
            World& _worldRef;
            pk::Sprite3DRenderable* _pSprite = nullptr;
            float _verticalOffset = 0.0f;

            // *NOTE! Below was bad idea, especially when object rotates in middle of anim..
            // Outermost vector[8] is for each different direction(N,NE,E...) of the sprite
            // Inner vector[varying] is for each animation of a single direction (as many anims we want)
            //std::vector<std::vector<pk::SpriteAnimator*>> _anims;
        public:
            VisualObject(pk::Scene& scene, World& worldRef, pk::Sprite3DRenderable* pSprite);
            VisualObject(const VisualObject& other);
            ~VisualObject();

            void assignAnimFrames(PK_ubyte tileObject, PK_ubyte tileAction, pk::Animation* anim);

            void show(
                PK_ubyte tileObject,
                PK_ubyte tileAction,
                int objDir,
                int camDir,
                const gamecommon::ObjectInfo& staticObjInfo,
                const VisualObjectInfo& visualObjInfo,
                float worldX,
                float worldZ,
                pk::Animation* animation,
                pk::vec3& tileMovement
            );
            void hide();

        private:
            void move(int dir, float speed, pk::vec3& tileMovement);
            void moveVertical(int dir, float speed, pk::vec3& tileMovement);
        };


        class ObjectInfoLib
        {
        private:
            static bool s_initialized;
            static std::vector<pk::web::WebTexture*> s_pTextures;
            // TODO: Unify object(properties) and visuals somehow together
            // *to prevent searching both separately..
            static std::vector<gamecommon::ObjectInfo> s_objects;
            static std::vector<VisualObjectInfo> s_objectVisuals;

        public:
            static gamecommon::ObjectInfo* get(int index);
            static VisualObjectInfo* getVisual(int index);
            static size_t get_size();
            static void create(const PK_byte* data, size_t dataSize);
            static void destroy();
        };
    }
}
