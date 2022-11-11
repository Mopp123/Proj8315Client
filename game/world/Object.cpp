
#include "Object.h"
#include <unordered_map>

#include "World.h"

using namespace pk;

namespace world
{
    namespace objects
    {
        size_t get_netw_objinfo_size()
        {
            size_t combinedStrLen = (OBJECT_DATA_STRLEN_NAME + OBJECT_DATA_STRLEN_DESCRIPTION);
            for (int i = 0; i < TILE_STATE_MAX_action + 1; ++i)
                combinedStrLen += OBJECT_DATA_STRLEN_ACTION_NAME;
            return combinedStrLen + 1;
        }


        // Returns "display direction" of a sprite depending ong the camera's direction
        static int get_display_dir(int objDir, int camDir)
        {
            const int directionCount = 8;
            const int dist = std::abs(objDir - camDir);
            if (objDir >= camDir)
                return dist;
            else
                return directionCount - dist;
        }
        
        // indexes: [objectType][action][animFrames]
        static std::vector<std::vector<std::vector<int>>> s_animationFrames = 
        {
            { 
                { 0 }
            },
            { 
                { 0 }
            },
            { 
                { 0 },
                { 1, 2 }
            },
            { 
                { 3 },
                { 0 },
                { 0, 1, 2, 3 }
            }
        };
        
        VisualObject::VisualObject(pk::Scene& scene, VisualWorld& worldRef, pk::Sprite3DRenderable* pSprite) :
            _worldRef(worldRef),
            _pSprite(pSprite)
        {}

        VisualObject::VisualObject(const VisualObject& other) :
            _worldRef(other._worldRef)
        {
            // Purposefully copying ptrs here and not their content!
            _pSprite = other._pSprite;
        }

        VisualObject::~VisualObject() 
        {}

        void VisualObject::assignAnimFrames(PK_ubyte tileObject, PK_ubyte tileAction, Animation* anim)
        {
            if (tileObject < s_animationFrames.size())
            {
                if (tileAction < s_animationFrames[tileObject].size())
                    anim->setFrames(s_animationFrames[tileObject][tileAction]);
            }
        }

        // TODO: Determine which sprite to show, depending on the "tileObject"
        // TODO: Sprite animating
        // TODO: Object speeds and stats
        void VisualObject::show(
            PK_ubyte tileObject, 
            PK_ubyte tileAction, 
            int objDir,
            int camDir,
            const ObjectInfo& staticObjInfo,
            float worldX, 
            float worldZ,
            pk::Animation* animation,
            pk::vec3& tileMovement
        )
        {
            _pSprite->setActive(true);
            _pSprite->texture = (Texture*)staticObjInfo.pTexture;

            const float currentTileHeight = _worldRef.getTileVisualHeightAt(
                worldX,
                worldZ
            );

            // NOTE: JUST FOR TESTING ATM
            if (tileObject == 3)
            {
                //animation->enableLooping(false);
                _pSprite->scale = vec2(8, 8);
            }
            else
            {
                //animation->enableLooping(true);
                _pSprite->scale = vec2(2, 2);
            }

            // NOTE: JUST TESTING ATM!
            // TODO: ..make it properly
            if (staticObjInfo.rotateableSprite)
            {
                int toDisplayDir = get_display_dir(objDir, camDir);
                vec2 texOffset = _pSprite->textureOffset;
                texOffset.y = (float)toDisplayDir;
                _pSprite->textureOffset = texOffset;
            }
            else
            {
                // This just temp here while testing rotateable sprites..
                _pSprite->textureOffset = vec2(0.0f, 0.0f);
            }

            PK_ubyte speedStat = staticObjInfo.speed;
            // If action == movement of some kind -> move the sprite
            switch (tileAction)
            {

                case TileStateAction::TILE_STATE_actionMove:
                    move(objDir, speedStat, tileMovement);
                    break;
                case TileStateAction::TILE_STATE_actionMoveVertical:
                    _verticalOffset = 15.0f;
                    moveVertical(objDir, speedStat * 5, tileMovement);
                    break;
                default:
                    break;
            }
            // JUST FOR TESTING: reset vertical offset, if someone had changed it for some reason..
            if (tileAction != TileStateAction::TILE_STATE_actionMoveVertical)
                _verticalOffset = 0.0f;
            
            // Animate sprite
            _pSprite->textureOffset.x = animation->getCurrentFrame();

            _pSprite->position.x = worldX + tileMovement.x;
            _pSprite->position.z = worldZ + tileMovement.z;
            // Update sprite's height depending on visual tile terrain 
            // (needs to be done after everything else so we get the "real accurate place"
            // -> for example all above code might change the sprite's "current visual tile")
            _pSprite->position.y = _worldRef.getTileVisualHeightAt(
                _pSprite->position.x, 
                _pSprite->position.z
            ) + _verticalOffset + tileMovement.y;
        }

        void VisualObject::hide()
        {
            _pSprite->setActive(false);
        }

        void VisualObject::move(int dir, float speed, pk::vec3& tileMovement)
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

            tileMovement.x += dirVec.x * speed * Timing::get_delta_time();
            tileMovement.z += dirVec.y * speed * Timing::get_delta_time();
        }

        void VisualObject::moveVertical(int dir, float speed, pk::vec3& tileMovement)
        {
            const float startHeight = 15.0f;
            // NOTE: north is considered up, south down
            if (dir == TileStateDirection::TILE_STATE_dirN)
                tileMovement.y += speed * 2.0f * Timing::get_delta_time();
            else if (dir == TileStateDirection::TILE_STATE_dirS)
                tileMovement.y -= speed * 2.0f * Timing::get_delta_time();
        }
    }
}
