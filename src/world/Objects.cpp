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
            pk::SkinnedRenderable* pSkinnedRenderable,
            pk::vec3 originalGridPos
        ) :
            _worldRef(worldRef),
            _entity(entity),
            _pStaticRenderable(pStaticRenderable),
            _pSkinnedRenderable(pSkinnedRenderable),
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
            // Display correct model
            // TODO: Make this shit better..
            Transform* pTransform = (Transform*)pScene->getComponent(
                _entity,
                ComponentType::PK_TRANSFORM
            );
            pTransform->setActive(true);
            Static3DRenderable* pStaticRenderable = (Static3DRenderable*)pScene->getComponent(
                _entity,
                ComponentType::PK_RENDERABLE_STATIC3D
            );
            SkinnedRenderable* pSkinnedRenderable = (SkinnedRenderable*)pScene->getComponent(
                _entity,
                ComponentType::PK_RENDERABLE_SKINNED
            );
            // Figure out should we render static or animated renderable
            if (tileObject == 2)
            {
                pSkinnedRenderable->meshID = visualObjInfo.pModel->getMesh(0)->getResourceID();
                pSkinnedRenderable->setActive(true);

                AnimationData* pAnimData = (AnimationData*)pScene->getComponent(
                    _entity,
                    ComponentType::PK_ANIMATION_DATA
                );
                pAnimData->setActive(true);
            }
            else
            {
                pStaticRenderable->meshID = visualObjInfo.pModel->getMesh(0)->getResourceID();
                pStaticRenderable->setActive(true);
            }

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
            mat4& tMat = pTransform->accessLocalTransformationMatrix();
            mat4& tMatGlobal = pTransform->accessTransformationMatrix();
            float& xPos = tMat[0 + 3 * 4];
            float& yPos = tMat[1 + 3 * 4];
            float& zPos = tMat[2 + 3 * 4];

            float& xPosGlobal = tMatGlobal[0 + 3 * 4];
            float& yPosGlobal = tMatGlobal[1 + 3 * 4];
            float& zPosGlobal = tMatGlobal[2 + 3 * 4];

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
            yPosGlobal = _worldRef.getTerrainHeight(
                xPosGlobal,
                zPosGlobal
            );// + _verticalOffset + tileMovement.y;
            //Debug::log("___TEST___SHOW obj height: " + std::to_string(yPos));

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
            Transform* pTransform = (Transform*)pScene->getComponent(
                _entity,
                ComponentType::PK_TRANSFORM
            );
            Static3DRenderable* pStaticRenderable = (Static3DRenderable*)pScene->getComponent(
                _entity,
                ComponentType::PK_RENDERABLE_STATIC3D
            );
            SkinnedRenderable* pSkinnedRenderable = (SkinnedRenderable*)pScene->getComponent(
                _entity,
                ComponentType::PK_RENDERABLE_SKINNED
            );
            AnimationData* pAnimData = (AnimationData*)pScene->getComponent(
                _entity,
                ComponentType::PK_ANIMATION_DATA
            );
            pTransform->setActive(false);
            pStaticRenderable->setActive(false);
            pSkinnedRenderable->setActive(false);
            pAnimData->setActive(false);
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


        std::vector<pk::Model*> ObjectInfoLib::s_models;
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

            // Test having just same model for all object types
            ImageData* pDefaultImage = resourceManager.loadImage(
                "assets/textures/default.jpg",
                true
            );
            ImageData* pTreeImage = resourceManager.loadImage(
                "assets/textures/tree1.png",
                true
            );
            ImageData* pUnitTexImage = resourceManager.loadImage(
                "assets/textures/characterTest.png",
                true
            );
            Texture_new* pDefaultTexture = resourceManager.createTexture(
                pDefaultImage->getResourceID(),
                defaultTextureSampler,
                true
            );
            Texture_new* pTreeTexture = resourceManager.createTexture(
                pTreeImage->getResourceID(),
                defaultTextureSampler,
                true
            );
            Texture_new* pUnitTexture = resourceManager.createTexture(
                pUnitTexImage->getResourceID(),
                defaultTextureSampler,
                true
            );
            Material* pDefaultMaterial = resourceManager.createMaterial(
                {pDefaultTexture->getResourceID()},
                0, // specular texture res id
                2.0f, // specular strength
                8.0f, // shininess
                0, // blendmap texture res id
                true
            );
            Material* pTreeMaterial = resourceManager.createMaterial(
                {pTreeTexture->getResourceID()},
                0, // specular texture res id
                0.0f, // specular strength
                1.0f, // shininess
                0, // blendmap texture res id
                true
            );
            Material* pUnitMaterial = resourceManager.createMaterial(
                {pUnitTexture->getResourceID()},
                0, // specular texture res id
                0.0f, // specular strength
                1.0f, // shininess
                0, // blendmap texture res id
                true
            );
            Model* pDefaultModel = resourceManager.loadModel(
                "assets/models/Cube.glb",
                pDefaultMaterial->getResourceID(),
                true
            );
            Model* pTreeModel1 = resourceManager.loadModel(
                "assets/models/tree1.glb",
                pTreeMaterial->getResourceID(),
                true
            );
            Model* pUnitModel = resourceManager.loadModel(
                "assets/models/characterTest.glb",
                pUnitMaterial->getResourceID()
            );

            s_models.push_back(pDefaultModel);
            s_models.push_back(pTreeModel1);
            s_models.push_back(pUnitModel);

            // TODO: Make below somehow more less dumb..
            // Set these objects' visual properties
            // *use 'i' since index of that list is equal to the object's type id
            for (int i = 0; i < s_objects.size(); ++i)
            {
                VisualObjectInfo visualObjInfo;

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
                        visualObjInfo.pModel = pTreeModel1;
                        break;
                    case 2:
                        visualObjInfo.rotateableSprite = true;
                        visualObjInfo.pModel = pUnitModel;
                        break;
                    case 3:
                        visualObjInfo.pModel = pDefaultModel;
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
            for (Model* pModel : s_models)
                resourceManager.deleteResource(pModel->getResourceID());
            s_initialized = false;
        }

        void ObjectInfoLib::set_objects_TESTING(const std::vector<gamecommon::ObjectInfo>& objects)
        {
            s_objects = objects;
        }

        std::string ObjectInfoLib::toString()
        {
            std::string output = "Object Info Library(" + std::to_string(s_objects.size()) + ")\n";
            for (int i = 0; i < s_objects.size(); ++i)
            {
                ObjectInfo& objInfo = s_objects[i];
                std::string objName(objInfo.name);
                std::string objDescription(objInfo.description);

                output += "  " + std::to_string(i) + ": " + objName + " -----------------\n";
                output += "    Description:\n      " + objDescription + "\n";
                output += "    Actions:\n";

                for (int j = 0; j < TILE_STATE_MAX_action; ++j)
                {
                    std::string action(objInfo.actionSlot[j]);
                    output += "      " + std::to_string(j) + ": " + action + "\n";
                }

                output += "    Stats:\n      Speed: " + std::to_string(objInfo.speed) + "\n\n";
            }
            return output;
        }

        pk::Model* ObjectInfoLib::get_default_static_model()
        {
            if (s_models.empty())
            {
                Debug::log(
                    "@ObjectInfoLib::get_default_static_model "
                    "No default static model assigned to required model slot: 0"
                    "ObjectInfoLib models was empty",
                    Debug::MessageType::PK_FATAL_ERROR
                );
                return nullptr;
            }
            return s_models[0];
        }

        pk::Model* ObjectInfoLib::get_default_rigged_model()
        {
            if (s_models.empty())
            {
                Debug::log(
                    "@ObjectInfoLib::get_default_rigged_model "
                    "No default rigged model assigned to required model slot: 2",
                    Debug::MessageType::PK_FATAL_ERROR
                );
                return nullptr;
            }
            return s_models[2];
        }
    }
}
