#include "MousePicker.h"
#include "core/input/InputEvent.h"
#include "utils/Algorithms.h"
#include "world/Objects.h"


using namespace pk;


namespace world
{
    using namespace objects;

    static vec3 get_midpoint(vec3 rayStartPos, vec3 ray, int recCount, const World * const pWorld)
    {
        vec3 halfRay = ray * 0.5f;
        vec3 midPoint = rayStartPos + halfRay;

        if (recCount <= 0)
        {
            return midPoint;
        }
        else
        {
            //float terrHeight = -terrain->getHeightAt(midPoint.x, midPoint.z);
            //float terrainHeight = getTileVisualHeightAt(midPoint.x, midPoint.z);
            float terrainHeight = pWorld->getTerrainHeight(midPoint.x, midPoint.z);
            if (midPoint.y < terrainHeight)
            {
                return get_midpoint(rayStartPos, halfRay, recCount - 1, pWorld);
            }
            else
            {
                return get_midpoint(midPoint, halfRay, recCount - 1, pWorld);
            }
        }
    }


	void MousePicker::PickerMouseButtonEvent::func(
        InputMouseButtonName button,
        InputAction action,
        int mods
    )
    {
        // NOTE: Atm need to set selected with right clicking as well for TileOptionsMenu to work properly
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT || button == InputMouseButtonName::PK_INPUT_MOUSE_RIGHT)
        {
            if (action == InputAction::PK_INPUT_PRESS && _clickState == 0)
            {
                int32_t obsSpaceX = 0;
                int32_t obsSpaceY = 0;
                _pMousePicker->getPickedObserveSpaceCoords(obsSpaceX, obsSpaceY);
                uint64_t tile = _pMousePicker->_pWorld->getTile(obsSpaceX, obsSpaceY);
                _pMousePicker->setSelectedTile(
                    tile,
                    _pMousePicker->_tileX,
                    _pMousePicker->_tileY
                );

                _clickState += 1;
            }
            else if (action == InputAction::PK_INPUT_RELEASE)
            {
                _clickState = 0;
            }
        }
    }


    void MousePicker::init(pk::Scene* pScene, World* pWorld)
    {
        if (!pScene)
        {
            Debug::log(
                "@MousePicker::init "
                "Attempted to init mousepicker but pScene was nullptr! ",
                Debug::MessageType::PK_FATAL_ERROR
            );
            return;
        }
        if (!pWorld)
        {
            Debug::log(
                "@MousePicker::init "
                "Attempted to init mousepicker but pWorld was nullptr! "
                "Make sure you have created World before calling this and make sure you provide valid pWorld",
                Debug::MessageType::PK_FATAL_ERROR
            );
            return;
        }

        _pScene = pScene;
        _pWorld = pWorld;

        _cursorEntity = _pScene->createEntity();
        Transform* pCursorTransform = pScene->createTransform(
            _cursorEntity,
            { 0, 1, 0 },
            { 0, 0, 0, 1 },
            { 1, 1, 1 }
        );
        ResourceManager& resManager = Application::get()->getResourceManager();
        ImageData* pImgData = resManager.loadImage("assets/textures/Cursor3D.png");
        TextureSampler texSampler;
        Texture* pCursorTexture = resManager.createTexture(
            pImgData->getResourceID(),
            texSampler
        );
        Material* pMaterial = resManager.createMaterial(
            {
                pCursorTexture->getResourceID()
            }
        );
        pMaterial->setShadeless(true);

        const float size = 2.0f;
        float vbData[32] = {
            -size, 0,-size,  0,1,0,  0,1,
            -size, 0, size,  0,1,0,  0,0,
             size, 0, size,  0,1,0,  1,0,
             size, 0,-size,  0,1,0,  1,1
        };
        unsigned short indices[6] =
        {
            0, 1, 3,
            3, 1, 2
        };

        Buffer* pVertexBuffer = Buffer::create(
            vbData,
            sizeof(float),
            32,
            BufferUsageFlagBits::BUFFER_USAGE_VERTEX_BUFFER_BIT,
            BufferUpdateFrequency::BUFFER_UPDATE_FREQUENCY_STATIC,
            false
        );
        Buffer* pIndexBuffer = Buffer::create(
            indices,
            sizeof(unsigned short),
            6,
            BufferUsageFlagBits::BUFFER_USAGE_INDEX_BUFFER_BIT,
            BufferUpdateFrequency::BUFFER_UPDATE_FREQUENCY_STATIC,
            false
        );

        Mesh* pCursorMesh = resManager.createMesh(
            { pVertexBuffer },
            pIndexBuffer,
            pMaterial->getResourceID()
        );

        Static3DRenderable* pCursorRenderable = _pScene->createStatic3DRenderable(
            _cursorEntity,
            pCursorMesh->getResourceID()
        );

        InputManager* pInputManager = Application::get()->accessInputManager();
        pInputManager->addMouseButtonEvent(new PickerMouseButtonEvent(this));
    }

    void MousePicker::update(bool clampToTile)
    {
        Camera* pCamera = (Camera*)_pScene->getComponent(_pScene->activeCamera, ComponentType::PK_CAMERA);
        Transform* pCamTransform = (Transform*)_pScene->getComponent(_pScene->activeCamera, ComponentType::PK_TRANSFORM);
        const mat4& projMat = pCamera->getProjMat3D();
        mat4 viewMat = pCamTransform->getTransformationMatrix();
        viewMat.inverse();

        // NOTE: Below could be done rather with CursorPosEvent?
        int mouseX = Application::get()->accessInputManager()->getMouseX();
        int mouseY = Application::get()->accessInputManager()->getMouseY();

        vec3 screenToWorldSpace = screen_to_world_space(mouseX, mouseY, projMat, viewMat);
        screenToWorldSpace.normalize();

        const mat4& camTMat = pCamTransform->getTransformationMatrix();
        vec3 startPos(camTMat[0 + 3 * 4], camTMat[1 + 3 * 4], camTMat[2 + 3 * 4]);

        const float maxPickingDist = 500.0f;
        const int maxPickRecursionCount = 500;

        // Terrain picking
        _worldCoords = get_midpoint(
            startPos,
            screenToWorldSpace * maxPickingDist,
            maxPickRecursionCount,
            _pWorld
        );

        _pWorld->worldToTileCoords(_worldCoords.x, _worldCoords.z, _tileX, _tileY);
        int observeSpaceTileX = 0;
        int observeSpaceTileY = 0;
        getPickedObserveSpaceCoords(observeSpaceTileX, observeSpaceTileY);

        if (clampToTile)
        {
            const float tileVisualScale = _pWorld->getTileVisualScale();
            _worldCoords.x = _tileX * tileVisualScale;
            uint64_t t = _pWorld->getTile(observeSpaceTileX, observeSpaceTileY);
            _worldCoords.y = ((float)gamecommon::get_tile_terrelevation(t)) + 0.125f;
            _worldCoords.z = _tileY * tileVisualScale;
        }

        Transform* pCursorTransform = (Transform*)_pScene->getComponent(
            _cursorEntity,
            ComponentType::PK_TRANSFORM
        );

        pCursorTransform->setPos({ _worldCoords.x, _worldCoords.y + 0.25f, _worldCoords.z });

        // Object picking
        // NOTE: Atm just testing here..
        // TODO: Finalize how this should be handled...
        std::vector<VisualObject>& tileObjects = _pWorld->accessVisualObjects();
        ResourceManager& resManager = Application::get()->getResourceManager();
        float prevObjDist = 99999.0f;
        VisualObject* pPickedObject = nullptr;
        for (int visibleObjIndex : _pWorld->getVisibleObjects())
        {
            VisualObject& obj = tileObjects[visibleObjIndex];
            obj.setColliderVisible(false); // Just testing here to indicate collision with displaying collider..

            const Transform* pColliderTransform = (const Transform*)_pScene->getComponent(
                obj.getColliderEntity(),
                ComponentType::PK_TRANSFORM
            );
            // just testing the collision here by setting material to red on collision..
            const Static3DRenderable* pColliderRenderable = (const Static3DRenderable*)_pScene->getComponent(
                obj.getColliderEntity(),
                ComponentType::PK_RENDERABLE_STATIC3D
            );
            Material* pColliderMaterial = ((Mesh*)resManager.getResource(pColliderRenderable->meshID))->accessMaterial();

            const vec3 colliderScale = pColliderTransform->getGlobalScale();
            vec3 colliderPos = pColliderTransform->getGlobalPos();
            // Need to add a bit to y, because "box origin" is at its' base and not its' center
            colliderPos.y += colliderScale.y;

            vec3 intersectionPoint(0, 0, 0);
	        bool collision = rayPolygonIntersect(
                startPos,
                screenToWorldSpace, // not sure is this actually the correct dir vec
	        	colliderPos,
	        	colliderScale,
	            intersectionPoint.x,
                intersectionPoint.y,
                intersectionPoint.z
	        );

            // Prevent picking all in the way of the ray.
            // Pick only the closest one to camera
            if (collision)
            {
                const mat4& colliderTMat = pColliderTransform->getTransformationMatrix();
                float dist = std::abs(camTMat[0 + 3 * 4] - colliderTMat[0 + 3 * 4]) + std::abs(camTMat[2 + 3 * 4] - colliderTMat[2 + 3 * 4]);
                if (dist < prevObjDist)
                    pPickedObject = &obj;
                prevObjDist = dist;
            }
        }
        if (pPickedObject)
            pPickedObject->setColliderVisible(true);
    }

    void MousePicker::set3DCursorVisible(bool arg)
    {
        Static3DRenderable* pRenderable = (Static3DRenderable*)_pScene->getComponent(
            _cursorEntity,
            ComponentType::PK_RENDERABLE_STATIC3D
        );
        pRenderable->setActive(arg);
    }

    void MousePicker::getPickedObserveSpaceCoords(int32_t& outX, int32_t& outY) const
    {
        const int observeAreaRadius = _pWorld->accessObserver().observeRadius;
        outX = _tileX - _pWorld->getTileX() + observeAreaRadius;
        outY = _tileY - _pWorld->getTileY() + observeAreaRadius;
    }

    void MousePicker::setSelectedTile(uint64_t tile, int tileX, int tileY)
    {
        _selectedTile = tile;
        _selectedTileX = tileX;
        _selectedTileY = tileY;
    }
}
