#include "Objects.h"
#include <unordered_map>
#include "World.h"
#include <string>
#include "../../Proj8315Common/src/messages/ObjMessages.h"


using namespace pk;
using namespace gamecommon;


namespace world
{
    namespace objects
    {
        VisualObjectInfo::TexturePortraitCropping VisualObjectInfo::s_defaultPortraitCropping;

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
                { 2 }
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


        static float dir_to_yaw(GC_ubyte dir)
        {
            const float dirAmount = M_PI / 4 * dir;
            return M_PI * 2.0f - dirAmount;
        }


        VisualObject::VisualObject(
            World& worldRef,
            entityID_t entity,
            pk::Static3DRenderable* pStaticRenderable,
            pk::vec3 originalGridPos
        ) :
            _worldRef(worldRef),
            _entity(entity),
            _pStaticRenderable(pStaticRenderable),
            _originalPos(originalGridPos)
        {}

        VisualObject::VisualObject(const VisualObject& other) :
            _worldRef(other._worldRef),
            _entity(other._entity),
            _originalPos(other._originalPos)
        {
            // Purposefully copying ptrs here and not their content!
            _pStaticRenderable = other._pStaticRenderable;
            _pSkinnedRenderable = other._pSkinnedRenderable;
            _pSprite = other._pSprite;
        }

        VisualObject::~VisualObject()
        {}

        /*
        void VisualObject::assignAnimFrames(PK_ubyte tileObject, PK_ubyte tileAction, Animation* anim)
        {
            if (tileObject < s_animationFrames.size())
            {
                if (tileAction < s_animationFrames[tileObject].size())
                    anim->setFrames(s_animationFrames[tileObject][tileAction]);
            }
        }
        */

        // TODO: Determine which sprite to show, depending on the "tileObject"
        // TODO: Sprite animating
        // TODO: Object speeds and stats
        void VisualObject::show(
            Scene* pScene,
            GC_ubyte tileObject,
            GC_ubyte tileAction,
            GC_ubyte objDir,
            //int camDir,
            const ObjectInfo& staticObjInfo,
            const VisualObjectInfo& visualObjInfo,
            float worldX,
            float worldZ,
            ////pk::Animation* animation,
            pk::vec3& tileMovement
        )
        {
            // Display correct model (Disable atm for testing)
            // Testing using just static model here..
            // TODO: Optimize below
            // NOTE: below fucks up if components pools resized!!
            //_pStaticRenderable->setActive(true);

            Static3DRenderable* pStaticRenderable = (Static3DRenderable*)pScene->getComponent(
                _entity,
                ComponentType::PK_RENDERABLE_STATIC3D
            );
            pStaticRenderable->meshID = visualObjInfo.pModel->getMesh(0)->getResourceID();
            pStaticRenderable->setActive(true);

            // BELOW NOT READY!
            /*

            //_pSprite->setActive(true);
            //_pSprite->texture = (Texture*)visualObjInfo.pTexture;

            // NOTE: JUST FOR TESTING ATM
            if (tileObject == 3)
            {
                //animation->enableLooping(false);
                _pSprite->scale = vec2(8, 8);
            }
            else if (tileObject == 2)
            {
                _pSprite->scale = vec2(1.5f, 1.5f);
            }
            else
            {
                //animation->enableLooping(true);
                _pSprite->scale = vec2(2, 2);
            }

            // NOTE: JUST TESTING ATM!
            // TODO: ..make it properly
            if (visualObjInfo.rotateableSprite)
            {
                int toDisplayDir = get_display_dir(objDir, camDir);
                vec2 texOffset = _pSprite->textureOffset;
                texOffset.y = (float)toDisplayDir;
                _pSprite->textureOffset = texOffset;
            }
            else
            {
                // This just temp here while testing rotateable sprites..
                //_pSprite->textureOffset = vec2(0.0f, 0.0f);
            }
            */

            GC_ubyte speedStat = staticObjInfo.speed;
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

            // NOTE: below may be a bit slow..
            Transform* pTransform = (Transform*)pScene->getComponent(
                _entity,
                ComponentType::PK_TRANSFORM
            );
            mat4& tMat = pTransform->accessLocalTransformationMatrix();
            float& xPos = tMat[0 + 3 * 4];
            float& yPos = tMat[1 + 3 * 4];
            float& zPos = tMat[2 + 3 * 4];
            // If not moving reset to original local pos (not sure if this fucks ups smthn..)
            /*
            if (tileAction != TileStateAction::TILE_STATE_actionMove)
            {
                xPos = _originalPos.x;
                zPos = _originalPos.z;
            }
            else
            {
                xPos += tileMovement.x;
                zPos += tileMovement.z;
            }
            */
            xPos = worldX + tileMovement.x;
            zPos = worldZ + tileMovement.z;
            yPos = _worldRef.getTerrainHeight(
                xPos,
                zPos
            );// + _verticalOffset + tileMovement.y;

            // Adjust facing direction.
            // Doing this only on y axis so no matrix multiplications required here
            const float yaw = dir_to_yaw(objDir);
            tMat[0 + 0 * 4] = std::cos(yaw);
            tMat[0 + 2 * 4] = std::sin(yaw);
            tMat[2 + 0 * 4] = -std::sin(yaw);
            tMat[2 + 2 * 4] = std::cos(yaw);

            // NOTE: old below when using just sprites..
            // Animate sprite
            //_pSprite->textureOffset.x = animation->getCurrentFrame();

            //_pSprite->position.x = worldX + tileMovement.x;
            //_pSprite->position.z = worldZ + tileMovement.z;
            // Update sprite's height depending on visual tile terrain
            // (needs to be done after everything else so we get the "real accurate place"
            // -> for example all above code might change the sprite's "current visual tile")
            //_pSprite->position.y = _worldRef.getTileVisualHeightAt(
            //    _pSprite->position.x,
            //    _pSprite->position.z
            //) + _verticalOffset + tileMovement.y;
        }

        void VisualObject::hide(pk::Scene* pScene)
        {
            //_pSprite->setActive(false);
            Static3DRenderable* pStaticRenderable = (Static3DRenderable*)pScene->getComponent(
                _entity,
                ComponentType::PK_RENDERABLE_STATIC3D
            );
            pStaticRenderable->setActive(false);
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


        bool ObjectInfoLib::s_initialized = false;
        //std::vector<Texture_new*> ObjectInfoLib::s_pTextures;
        std::vector<ObjectInfo> ObjectInfoLib::s_objects;
        std::vector<VisualObjectInfo> ObjectInfoLib::s_objectVisuals;

        ObjectInfo* ObjectInfoLib::get(int index)
        {
            if (index < 0 || index >= s_objects.size())
            {
                Debug::log(
                    "Attempted to access invalid index of Object Info lib (Object properties). Index: " + std::to_string(index) + " Info lib size was: " + std::to_string(s_objects.size()),
                    Debug::MessageType::PK_ERROR);
                return nullptr;
            }
            return &s_objects[index];
        }

        VisualObjectInfo* ObjectInfoLib::getVisual(int index)
        {
            if (index < 0 || index >= s_objectVisuals.size())
            {
                Debug::log(
                    "Attempted to access invalid index of Object Info lib (Object visuals). Index: " + std::to_string(index) + " Info lib size was: " + std::to_string(s_objects.size()),
                    Debug::MessageType::PK_ERROR);
                return nullptr;
            }
            return &s_objectVisuals[index];
        }

        size_t ObjectInfoLib::get_size()
        {
            return s_objects.size();
        }

        void ObjectInfoLib::create(const PK_byte* pData, size_t dataSize)
        {
            if (s_initialized)
            {
                Debug::log(
                    "Attempted to create Object Info lib while previous one still exists",
                    Debug::MessageType::PK_ERROR
                );
                return;
            }

            ObjInfoLibResponse objInfoMsg(pData, dataSize);
            s_objects = objInfoMsg.getObjects();

            create_object_visuals();
        }

        void ObjectInfoLib::create_object_visuals()
        {
            if (s_objects.empty())
            {
                Debug::log(
                    "@ObjectInfoLib::create_object_visuals "
                    "No objects assigned",
                    Debug::MessageType::PK_FATAL_ERROR
                );
                return;
            }

            ResourceManager& resourceManager = Application::get()->getResourceManager();
            TextureSampler defaultTextureSampler;

            for (int i = 0; i < s_objects.size(); ++i)
            {
                VisualObjectInfo visualObjInfo;
                // TODO: Make below somehow more less dumb..
                // Set these objects' visual properties
                // *use 'i' since index of that list is equal to the object's type id

                // Test having just same model for all object types
                ImageData* pImg = resourceManager.loadImage(
                    "assets/textures/default.jpg",
                    true
                );
                Texture_new* pTexture = resourceManager.createTexture(
                    pImg->getResourceID(),
                    defaultTextureSampler,
                    true
                );
                Material* pMaterial = resourceManager.createMaterial(
                    {pTexture->getResourceID()},
                    0, // specular texture res id
                    2.0f, // specular strength
                    8.0f, // shininess
                    0, // blendmap texture res id
                    true
                );
                Model* pModel0 = resourceManager.loadModel(
                    "assets/models/Cube.glb",
                    pMaterial->getResourceID(),
                    true
                );
                Model* pModel1 = resourceManager.loadModel(
                    "assets/models/Arrow.glb",
                    pMaterial->getResourceID(),
                    true
                );

                // Below old way of using sprites for everything..
                // NOTE: Also texture loading has changed
                //  -> below should NOT BE USED anymore!
                //TextureSampler spriteTextureSampler =
                //{
                //    TextureSamplerFilterMode::PK_SAMPLER_FILTER_MODE_LINEAR,
                //    TextureSamplerAddressMode::PK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                //    2
                //};
                // NOTE: No fucking idea why load new textures for each object..
                //s_pTextures.push_back(new WebTexture("assets/environment.png", spriteTextureSampler, 8));
                //s_pTextures.push_back(new WebTexture("assets/MovementTest.png", spriteTextureSampler, 8));
                //s_pTextures.push_back(new WebTexture("assets/landings.png", spriteTextureSampler, 4));

                switch (i)
                {
                    case 1:
                        visualObjInfo.pModel = pModel0;
                        break;
                    case 2:
                        visualObjInfo.rotateableSprite = true;
                        visualObjInfo.pModel = pModel1;
                        break;
                    case 3:
                        visualObjInfo.pModel = pModel0;
                        break;
                    default:
                        break;
                }
                s_objectVisuals.push_back(visualObjInfo);
            }
        }

        void ObjectInfoLib::destroy()
        {
            ResourceManager& resourceManager = Application::get()->getResourceManager();
            for (Model* pModel : s_pModels)
                resourceManager.deleteResource(pModel->getResourceID());
            s_initialized = false;
        }

        void ObjectInfoLib::set_objects_TESTING(const std::vector<gamecommon::ObjectInfo>& objects)
        {
            s_objects = objects;
        }
    }
}