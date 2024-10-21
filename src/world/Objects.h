#pragma once

#include <vector>
#include <unordered_map>

#include "../../PortablePesukarhu/ppk.h"
#include "Tile.h"
#include "../../Proj8315Common/src/Object.h"
#include "ecs/components/renderable/SkinnedRenderable.h"
#include "ecs/components/renderable/Static3DRenderable.h"


namespace world
{
    class World;

    namespace objects
    {
        // NEW
        class VisualObjectInfo
        {
        public:
            // NOTE: atm using models for all objects instead of sprites
            pk::Model* pModel = nullptr; // *Not owned by VisualObjectInfo!
            //pk::Texture_new* pTexture = nullptr; // *NOTE! ObjectInfo doesn't own its' texture

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
                pModel(other.pModel),
                //pTexture(other.pTexture),
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
            entityID_t _entity = NULL_ENTITY_ID;
            pk::Static3DRenderable* _pStaticRenderable = nullptr;
            pk::SkinnedRenderable* _pSkinnedRenderable = nullptr;
            pk::AnimationData* _pAnimData = nullptr;

            const size_t _colliderVertexCount = 8;
            pk::vec3 _colliderVertices[8];
            // Mesh and entity for debugging collision box
            static PK_id s_colliderModelID;
            entityID_t _colliderEntity = 0;

            // NOTE: Current engine version doesn't support sprite rendering
            // TODO: Add sprite renderer to engine
            pk::Sprite3DRenderable* _pSprite = nullptr;
            pk::vec3 _originalPos; // original local pos in grid
            float _verticalOffset = 0.0f;

            // *NOTE! Below was bad idea, especially when object rotates in middle of anim..
            // Outermost vector[8] is for each different direction(N,NE,E...) of the sprite
            // Inner vector[varying] is for each animation of a single direction (as many anims we want)
            //std::vector<std::vector<pk::SpriteAnimator*>> _anims;
        public:
            static std::vector<pk::vec3> s_colliderSizes;

            VisualObject(
                World& worldRef,
                entityID_t entity,
                pk::Static3DRenderable* pStaticRenderable,
                pk::SkinnedRenderable* pSkinnedRenderable,
                pk::vec3 originalGridPos
            );

            VisualObject(const VisualObject& other);
            ~VisualObject();

            // NOTE: current anim system is fucked!
            //void assignAnimFrames(PK_ubyte tileObject, PK_ubyte tileAction, pk::Animation* anim);

            void show(
                pk::Scene* pScene,
                GC_ubyte tileObject,
                GC_ubyte tileAction,
                GC_ubyte objDir,
                //int camDir,
                const gamecommon::ObjectInfo& staticObjInfo,
                const VisualObjectInfo& visualObjInfo,
                float worldX,
                float worldZ,
                ////pk::Animation* animation,
                pk::vec3& tileMovement
            );
            void hide(pk::Scene* pScene);

            void setColliderVisible(bool arg);

            inline entityID_t getEntity() const { return _entity; }
            inline entityID_t getColliderEntity() const { return _colliderEntity; }
            inline const pk::vec3& getOriginalPos() const { return _originalPos;  }

        private:
            void move(int dir, float speed, pk::vec3& tileMovement);
            void moveVertical(int dir, float speed, pk::vec3& tileMovement);
        };


        class ObjectInfoLib
        {
        private:
            static bool s_initialized;
            // For now all objects are 3d models..
            //static std::vector<pk::Texture_new*> s_pTextures;
            static std::vector<pk::Model*> s_models;


            // TODO: Unify object(properties) and visuals somehow together
            // *to prevent searching both separately..
            static std::vector<gamecommon::ObjectInfo> s_objects;
            static std::vector<VisualObjectInfo> s_objectVisuals;

        public:
            static gamecommon::ObjectInfo* get(int index);
            static VisualObjectInfo* getVisual(int index);
            static size_t get_size();
            static void create(const PK_byte* pData, size_t dataSize);
            static void create_object_visuals();
            static void destroy();

            static void set_objects_TESTING(const std::vector<gamecommon::ObjectInfo>& objects);

            static std::string toString();

            static pk::Model* get_default_static_model();
            // TODO: better way to specify default models
            //  -> atm these are hardcoded!!
            static pk::Model* get_default_rigged_model();
        };
    }
}
