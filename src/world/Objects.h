#pragma once

#include <vector>
#include <unordered_map>

#include "pesukarhu/ppk.h"
#include "Tile.h"
#include "../../Proj8315Common/src/Object.h"


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
            // *Not owned by VisualObjectInfo!
            pk::Model* pModel = nullptr;
            // *Atm shit gets fucked if changing scale of the actual model, since modifying
            // y axis rotation directly from its' transformationMatrix
            // TODO: Maybe fix that?
            pk::vec3 colliderScale = pk::vec3(1, 1, 1);

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

            VisualObjectInfo() {}
            VisualObjectInfo(const VisualObjectInfo& other):
                pModel(other.pModel),
                colliderScale(other.colliderScale),
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
                const float tileVisualScale,
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
            static std::vector<pk::Model*> s_models;

            // TODO: Unify object(properties) and visuals somehow together
            // *to prevent searching both separately..
            static std::vector<gamecommon::ObjectInfo> s_objects;
            static std::vector<VisualObjectInfo> s_objectVisuals;

            static pk::Model* s_defaultStaticModel;
            static pk::Model* s_defaultRiggedModel;

        public:
            static gamecommon::ObjectInfo* get(int index);
            static std::vector<gamecommon::ObjectInfo>& getObjectInfos();
            static VisualObjectInfo* get_visual(int index);
            static size_t get_size();
            static void create(const PK_byte* pData, size_t dataSize);
            // Loads models, textures and other properties to display objects
            static bool load_object_visuals();
            static void destroy();

            static void set_objects_TESTING(const std::vector<gamecommon::ObjectInfo>& objects);

            static std::string to_string();

            static pk::Model* get_default_static_model();
            // TODO: better way to specify default models
            //  -> atm these are hardcoded!!
            static pk::Model* get_default_rigged_model();
        };
    }
}
