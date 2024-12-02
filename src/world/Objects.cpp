#include "Objects.h"
#include <unordered_map>
#include "World.h"
#include <string>
#include "../../Proj8315Common/src/messages/ObjMessages.h"
#include "../PortablePesukarhu/json.hpp"
#include <fstream>


using namespace pk;
using namespace gamecommon;


namespace world
{
    namespace objects
    {
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

        static float dir_to_yaw(GC_ubyte dir)
        {
            const float dirAmount = M_PI / 4 * dir;
            return M_PI * 2.0f - dirAmount;
        }


        PK_id VisualObject::s_colliderModelID = 0;
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
        {
            if (entity == NULL_ENTITY_ID)
            {
                Debug::log(
                    "@VisualObject::VisualObject "
                    "Provided entityID was NULL_ENTITY_ID",
                    Debug::MessageType::PK_FATAL_ERROR
                );
                return;
            }

            // Size is eventually determined by object type
            const float defaultColliderSize = 2.0f;

            ResourceManager& resManager = Application::get()->getResourceManager();
            const Model* pColliderModel = nullptr;
            if (s_colliderModelID == 0)
            {
                pColliderModel = resManager.loadModel(
                    "assets/models/UnitCubeOriginBottom.glb",
                    resManager.getDefaultMaterial()->getResourceID(),
                    true
                );
                s_colliderModelID = pColliderModel->getResourceID();
            }
            else
            {
                pColliderModel = (const Model*)resManager.getResource(s_colliderModelID);
            }

            Scene* pScene = Application::get()->accessCurrentScene();
            _colliderEntity = pScene->createEntity();
            Transform::create(
                _colliderEntity,
                { 0, 0, 0 },
                { 0, 0, 0, 1 },
                { defaultColliderSize, defaultColliderSize, defaultColliderSize }
            );
            Static3DRenderable::create(
                _colliderEntity,
                pColliderModel->getMesh(0)->getResourceID()
            );
            pScene->addChild(_entity, _colliderEntity);
            setColliderVisible(false);
        }

        VisualObject::VisualObject(const VisualObject& other) :
            _worldRef(other._worldRef),
            _entity(other._entity),
            _colliderEntity(other._colliderEntity),
            _originalPos(other._originalPos)
        {
            // Purposefully copying ptrs here and not their content!
            _pStaticRenderable = other._pStaticRenderable;
            _pSkinnedRenderable = other._pSkinnedRenderable;
            _pSprite = other._pSprite;
        }

        VisualObject::~VisualObject()
        {
        }

        // TODO: Determine which sprite to show, depending on the "tileObject"
        // TODO: Sprite animating
        // TODO: Object speeds and stats
        void VisualObject::show(
            Scene* pScene,
            const float tileVisualScale,
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
            Transform* pTransform = (Transform*)pScene->getComponent(
                _entity,
                ComponentType::PK_TRANSFORM
            );
            pTransform->setActive(true);

            const Mesh* pUseMesh = visualObjInfo.pModel->getMesh(0);
            // figure out which renderable to use...
            // atm quite shitty way...
            bool useRiggedMesh = !pUseMesh->getBindPose().joints.empty();

            // Display correct model
            // TODO: Make this shit better..
            Static3DRenderable* pStaticRenderable = (Static3DRenderable*)pScene->getComponent(
                _entity,
                ComponentType::PK_RENDERABLE_STATIC3D
            );
            SkinnedRenderable* pSkinnedRenderable = (SkinnedRenderable*)pScene->getComponent(
                _entity,
                ComponentType::PK_RENDERABLE_SKINNED
            );

            if (useRiggedMesh)
            {
                pSkinnedRenderable->meshID = pUseMesh->getResourceID();
                pSkinnedRenderable->setActive(true);
                pStaticRenderable->setActive(false);

                AnimationData* pAnimData = (AnimationData*)pScene->getComponent(
                    _entity,
                    ComponentType::PK_ANIMATION_DATA
                );
                pAnimData->setActive(true);
            }
            else
            {
                pStaticRenderable->meshID = pUseMesh->getResourceID();
                pStaticRenderable->setActive(true);
                pSkinnedRenderable->setActive(false);
            }

            const float visualTileSizeModifier = tileVisualScale * 0.5f;
            float speedValue = ((float)staticObjInfo.speed) * visualTileSizeModifier;
            // If action == movement of some kind -> move the sprite
            switch (tileAction)
            {
                case TileStateAction::TILE_STATE_actionMove:
                    move(objDir, speedValue, tileMovement);
                    break;
                case TileStateAction::TILE_STATE_actionMoveVertical:
                    _verticalOffset = 15.0f;
                    moveVertical(objDir, speedValue * 5, tileMovement);
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

            // Adjust facing direction.
            // Doing this only on y axis so no matrix multiplications required here
            const float yaw = dir_to_yaw(objDir);
            tMat[0 + 0 * 4] = std::cos(yaw);
            tMat[0 + 2 * 4] = std::sin(yaw);
            tMat[2 + 0 * 4] = -std::sin(yaw);
            tMat[2 + 2 * 4] = std::cos(yaw);

            // Adjust collider transform
            Transform* pColliderTransform = (Transform*)pScene->getComponent(
                _colliderEntity,
                ComponentType::PK_TRANSFORM
            );
            pColliderTransform->setScale(visualObjInfo.colliderScale);
            mat4& colliderMatGlobal = pColliderTransform->accessTransformationMatrix();
            mat4& colliderMatLocal = pColliderTransform->accessLocalTransformationMatrix();
            colliderMatGlobal[0 + 3 * 4] = xPosGlobal;
            colliderMatGlobal[1 + 3 * 4] = yPosGlobal;
            colliderMatGlobal[2 + 3 * 4] = zPosGlobal;

            //setColliderVisible(true);
        }

        void VisualObject::hide(pk::Scene* pScene)
        {
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
            setColliderVisible(false);
        }

        void VisualObject::setColliderVisible(bool arg)
        {
            Scene* pScene = Application::get()->accessCurrentScene();
            Static3DRenderable* pColliderRenderable = (Static3DRenderable*)pScene->getComponent(
                _colliderEntity,
                ComponentType::PK_RENDERABLE_STATIC3D
            );
            pColliderRenderable->setActive(arg);
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

            // Don't know what was the point in below since dirVec already
            // is "scaled" for moving more diagonally if not normalizing it
            //  -> atm it seems to work at least almost perfectly
            //dirVec.normalize();
            //if (diag)
            //    dirVec = dirVec * 1.4f;

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
        std::vector<ObjectInfo> ObjectInfoLib::s_objects;
        std::vector<VisualObjectInfo> ObjectInfoLib::s_objectVisuals;
        Model* ObjectInfoLib::s_defaultStaticModel = nullptr;
        Model* ObjectInfoLib::s_defaultRiggedModel = nullptr;

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

        std::vector<gamecommon::ObjectInfo>& ObjectInfoLib::getObjectInfos()
        {
            return s_objects;
        }

        VisualObjectInfo* ObjectInfoLib::get_visual(int index)
        {
            if (index < 0 || index >= s_objectVisuals.size())
            {
                Debug::log(
                    "Attempted to access invalid index of Object Info lib (Object visuals). Index: " + std::to_string(index) + " Info lib size was: " + std::to_string(s_objectVisuals.size()),
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

            if (s_objects.empty())
            {
                Debug::log(
                    "@ObjectInfoLib::create_object_visuals "
                    "No objects assigned",
                    Debug::MessageType::PK_FATAL_ERROR
                );
                return;
            }
            load_object_visuals();
        }


        // Loads models, textures and other properties to display objects
        // TODO: Evantually make filepath modifyable from somewhere..
        bool ObjectInfoLib::load_object_visuals()
        {
            std::string rootAssetFilepath = "assets/";
            std::string objPropertiesPath = rootAssetFilepath + "ObjectVisualProperties.json";
            std::string modelsPath = rootAssetFilepath + "models/";
            std::string texturesPath = rootAssetFilepath + "textures/";

            Debug::log(
                "@ObjectInfoLib::load_object_visuals "
                "Reading object visual properties from file: " + objPropertiesPath + " "
                "WARNING: If platform is web on json parse error, no sensible error message "
                "gets thrown. If crashing occurs after this message make sure the file is "
                "valid json!"
            );

            std::ifstream propertiesFile;
            propertiesFile.open(objPropertiesPath);

            if (!propertiesFile.is_open())
            {
                Debug::log(
                    "@ObjectInfoLib::load_object_visuals "
                    "Failed to open object visual properties file from: " + objPropertiesPath + " "
                    "Make sure you entered correct filepath!",
                    Debug::MessageType::PK_FATAL_ERROR
                );
                return false;
            }

            // Relying here that the file is valid json... i know fucking dumb
            // but the fucking lib doesn't offer other error handling that try-catches
            // so... it is what it is for now...
            // TODO: Figure out some better way of dealing with this!
            nlohmann::json jsonData = nlohmann::json::parse(propertiesFile);

            Debug::log(
                "@ObjectInfoLib::load_object_visuals "
                "Loading textures and creating materials..."
            );

            ResourceManager& resourceManager = Application::get()->getResourceManager();

            TextureSampler defaultTextureSampler;
            std::vector<Material*> materials;
            for (auto& materialData  : jsonData["materials"])
            {
                const std::string texturePath = texturesPath + materialData["texture"].get<std::string>();
                Texture* pTexture = resourceManager.loadTexture(
                    texturePath,
                    defaultTextureSampler,
                    true
                );
                if (!pTexture)
                {
                    Debug::log(
                        "@ObjectInfoLib::load_object_visuals "
                        "Failed to load texture from: " + texturePath,
                        Debug::MessageType::PK_FATAL_ERROR
                    );
                    propertiesFile.close();
                    return false;
                }
                Material* pMaterial = resourceManager.createMaterial(
                    { pTexture->getResourceID() },
                    0, // specular texture res id
                    0.0f, // specular strength
                    1.0f, // shininess
                    { 1, 1, 1, 1 }, // color
                    false, // shadeless
                    true // persistent
                );
                if (!pMaterial)
                {
                    Debug::log(
                        "@ObjectInfoLib::load_object_visuals "
                        "Failed to create material using texture: " + texturePath,
                        Debug::MessageType::PK_FATAL_ERROR
                    );
                    propertiesFile.close();
                    return false;
                }
                materials.push_back(pMaterial);
                Debug::log("    Created material successfully using texture: " + texturePath);
            }

            Debug::log(
                "@ObjectInfoLib::load_object_visuals "
                "Loading models..."
            );

            std::vector<Model*> models;
            for (auto& modelProperties : jsonData["models"])
            {
                const std::string filepath = modelsPath + modelProperties["file"].get<std::string>();
                const int materialIndex = modelProperties["material"];

                if (materialIndex < 0 || materialIndex >= materials.size())
                {
                    Debug::log(
                        "@ObjectInfoLib::load_object_visuals "
                        "Invalid material index: " + std::to_string(materialIndex) + " "
                        "for model: " + filepath + " "
                        "available materials count: " + std::to_string(materials.size()),
                        Debug::MessageType::PK_FATAL_ERROR
                    );
                    propertiesFile.close();
                    return false;
                }

                Model* pModel = resourceManager.loadModel(
                    filepath,
                    materials[materialIndex]->getResourceID(),
                    true
                );

                // Set default models if not set already
                const std::string modelType = modelProperties["type"].get<std::string>();
                if (!s_defaultStaticModel && modelType == "static")
                    s_defaultStaticModel = pModel;
                if (!s_defaultRiggedModel && modelType == "rigged")
                    s_defaultRiggedModel = pModel;

                if (!pModel)
                {
                    Debug::log(
                        "@ObjectInfoLib::load_object_visuals "
                        "Failed to load model from: " + filepath,
                        Debug::MessageType::PK_FATAL_ERROR
                    );
                    propertiesFile.close();
                    return false;
                }
                models.push_back(pModel);
                Debug::log("    Loaded model successfully from: " + filepath + " using material index: " + std::to_string(materialIndex));
            }

            Debug::log(
                "@ObjectInfoLib::load_object_visuals "
                "Loading object properties..."
            );
            // create VisualObjInfo instances
            int objectIndex = 0;
            for (auto& objProperties : jsonData["objects"])
            {
                const int modelIndex = objProperties["model"];
                if (modelIndex < 0 || modelIndex >= models.size())
                {
                    Debug::log(
                        "@ObjectInfoLib::load_object_visuals "
                        "Invalid model index: " + std::to_string(modelIndex) + " "
                        "for object index: " + std::to_string(objectIndex),
                        Debug::MessageType::PK_FATAL_ERROR
                    );
                    propertiesFile.close();
                    return false;
                }

                std::vector<float> colliderScale = objProperties["scale"].get<std::vector<float>>();
                if (colliderScale.size() != 3)
                {
                    Debug::log(
                        "@ObjectInfoLib::load_object_visuals "
                        "Invalid component count for object scale: " + std::to_string(colliderScale.size()) + " "
                        "at index: " + std::to_string(objectIndex) + " "
                        "required count is 3",
                        Debug::MessageType::PK_FATAL_ERROR
                    );
                    propertiesFile.close();
                    return false;
                }

                VisualObjectInfo visualInfo;
                visualInfo.pModel = models[modelIndex];
                visualInfo.colliderScale = vec3(
                    colliderScale[0],
                    colliderScale[1],
                    colliderScale[2]
                );
                s_objectVisuals.push_back(visualInfo);
                ++objectIndex;

                Debug::log("    Created visual object info successfully");
            }

            propertiesFile.close();
            return true;
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

        std::string ObjectInfoLib::to_string()
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
            if (!s_defaultStaticModel)
            {
                Debug::log(
                    "@ObjectInfoLib::get_default_static_model "
                    "No default static model assigned!",
                    Debug::MessageType::PK_FATAL_ERROR
                );
            }
            return s_defaultStaticModel;
        }

        pk::Model* ObjectInfoLib::get_default_rigged_model()
        {
            if (!s_defaultRiggedModel)
            {
                Debug::log(
                    "@ObjectInfoLib::get_default_rigged_model "
                    "No default rigged model assigned!",
                    Debug::MessageType::PK_FATAL_ERROR
                );
            }
            return s_defaultRiggedModel;
        }
    }
}
