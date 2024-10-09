#include "InGameLocal.h"
#include "Object.h"
#include "Tile.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "MainMenu.h"
#include "core/input/InputEvent.h"
#include "world/Objects.h"


using namespace pk;
using namespace net;

using namespace gamecommon;


static float s_idleAnimSpeed = 0.75f;
static std::vector<uint32_t> s_idleAnimFrames = {
    1, 6
};

static float s_moveAnimSpeed = 10.0f;
static std::vector<uint32_t> s_moveAnimFrames = {
    11, 16, 21, 26
};

static void get_world_state(int xPos, int zPos, int observeRadius, std::vector<uint64_t>& target, const std::vector<uint64_t>& worldMap, int worldWidth)
{
    int writePos = 0;
    memset(target.data(), 0, sizeof(uint64_t) * target.size());

    for(int z = zPos - observeRadius; z <= zPos + observeRadius; ++z)
    {
        for(int x = xPos - observeRadius; x <= xPos + observeRadius; ++x)
        {
            // Make sure coords are valid tile coords
            if(x >= 0 && x < worldWidth && z >= 0 && z < worldWidth)
            {
                // Should never go out of range? since prev if?.
                int tileIndex = x + z * worldWidth;
                target[writePos] = worldMap[tileIndex];
            }
            writePos += 1;
        }
    }
}


InGameLocal::InGameLocal()
{}

InGameLocal::~InGameLocal()
{
    delete _pCamController;
}

void InGameLocal::init()
{
    initBase();

    // Set clear color
    Application::get()->getMasterRenderer().setClearColor({ 0, 0, 0, 1});

    _pCamController = new CameraController(activeCamera, 10.0f);
    Transform* pCamTransform = (Transform*)getComponent(activeCamera, ComponentType::PK_TRANSFORM);

    // Temporarely create ObjInfoLib locally
    gamecommon::ObjectInfo emptyObj(
        "Empty",
        "",
        {
            "Idle"
        },
        0,
        0
    );
    uint64_t staticObjState = 0;
    set_tile_thingid(staticObjState, 1);
    gamecommon::ObjectInfo staticObj(
        "Static Object",
        "Doesn't do anything",
        {
            "Idle"
        },
        0,
        staticObjState
    );
    uint64_t movingObjState = 0;
    set_tile_thingid(movingObjState, 2);
    gamecommon::ObjectInfo movingObj(
        "Moving Object",
        "Capable of moving",
        {
            "Idle",
            "Move"
        },
        1,
        movingObjState
    );
    world::objects::ObjectInfoLib::set_objects_TESTING(
        {
            emptyObj,
            staticObj,
            movingObj
        }
    );
    Debug::log("___TEST___created obj info lib:\n" + world::objects::ObjectInfoLib::toString());

    world::objects::ObjectInfoLib::create_object_visuals();

    // World can be created only after object info lib creation
    _pWorld = new world::World(
        (Scene&)(*this),
        pCamTransform,
        _observeAreaRadius
    );


    _testMapFull.resize(_testMapWidth * _testMapWidth * sizeof(uint64_t), 0);

    /*
    int areaWidth = _testMapWidth;
    for (int y = 0; y < areaWidth; ++y)
    {
        for (int x = 0; x < areaWidth; ++x)
        {
            int randTerrainType = std::rand() % 2;
            set_tile_terrtype(_testMapFull[x + y * _testMapWidth], randTerrainType);
            int randHeight = std::rand() % 3;
            set_tile_terrelevation(_testMapFull[x + y * _testMapWidth], 1 + randHeight);
            bool plantTree = (std::rand() % 4) == 3;

            set_tile_thingid(_testMapFull[x + y * _testMapWidth], (GC_ubyte)plantTree);
        }
    }*/

    set_tile_terrelevation(_testMapFull[10 + 12 * _testMapWidth], 0);
    set_tile_thingid(_testMapFull[1 + 1 * _testMapWidth], 2);
    set_tile_thingid(_testMapFull[3 + 5 * _testMapWidth], 2);
    set_tile_thingid(_testMapFull[9 + 10 * _testMapWidth], 2);
    set_tile_thingid(_testMapFull[12 + 4 * _testMapWidth], 2);

    _testMapLocal.resize(_observeAreaWidth * _observeAreaWidth * sizeof(uint64_t), 0);

    //get_world_state(0, 0, _observeAreaRadius, _testMapLocal, _testMapFull, _testMapWidth);
    //_pWorld->updateObservedArea(_testMapLocal.data());


    // TESTING ANIM LOADING
    /*
    pk::ResourceManager& resourceManager = pk::Application::get()->getResourceManager();
    pk::ImageData* pTestImg = resourceManager.loadImage("assets/textures/characterTest.png");
    pk::TextureSampler defaultTexSampler;
    pk::Texture_new* pTestTexture = resourceManager.createTexture(
        pTestImg->getResourceID(),
        defaultTexSampler
    );
    pk::Material* pMaterial = resourceManager.createMaterial(
        { pTestTexture->getResourceID() },
        0,
        0.0f,
        1.0f
    );
    Model* pAnimModel = resourceManager.loadModel(
        "assets/models/characterTest.glb",
        pMaterial->getResourceID()
    );
    Mesh* pAnimMesh = pAnimModel->accessMesh(0);
    Animation* pTestAnimation = resourceManager.createAnimation(
        pAnimMesh->accessBindPose(),
        pAnimMesh->accessAnimPoses()
    );

    entityID_t animatedEntity = createEntity();

    createTransform(animatedEntity, { 0, 5.0f, 0 }, { 1.0f, 1.0f, 1.0f }, 0.0f, 0.0f);

    _rootJointEntity = createSkeletonEntity(animatedEntity, pAnimMesh->accessBindPose());

    createSkinnedRenderable(
        animatedEntity,
        pAnimModel->getResourceID(),
        pAnimMesh->getResourceID(),
        _rootJointEntity
    );
    createAnimationData(
        _rootJointEntity,
        ((Resource*)pTestAnimation)->getResourceID(),
        AnimationMode::PK_ANIMATION_MODE_REPEAT,
        s_idleAnimSpeed,
        s_idleAnimFrames
    );
    */
}


static float s_updateTimer = 0.0f;
static float s_maxUpdateTimer = 0.5f;

void InGameLocal::update()
{
    _pCamController->update();

    InputManager* pInputManager = Application::get()->accessInputManager();
    // test change animation
    /*
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_F))
    {
        Scene* pScene = (Scene*)this;
        AnimationData* pAnimData = (AnimationData*)(pScene->getComponent(
            _rootJointEntity,
            ComponentType::PK_ANIMATION_DATA
        ));
        pAnimData->set(
            s_idleAnimFrames,
            s_idleAnimSpeed,
            AnimationMode::PK_ANIMATION_MODE_REPEAT
        );
    }
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_G))
    {
        Scene* pScene = (Scene*)this;
        AnimationData* pAnimData = (AnimationData*)(pScene->getComponent(
            _rootJointEntity,
            ComponentType::PK_ANIMATION_DATA
        ));
        pAnimData->set(
            s_moveAnimFrames,
            s_moveAnimSpeed,
            AnimationMode::PK_ANIMATION_MODE_REPEAT
        );
    }
    */

    // test update tile dir
    int testTileIndex = 10 + 12 * _testMapWidth;
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_1))
    {
        set_tile_facingdir(
            _testMapFull[testTileIndex],
            TileStateDirection::TILE_STATE_dirN
        );
    }
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_2))
    {
        set_tile_facingdir(
            _testMapFull[testTileIndex],
            TileStateDirection::TILE_STATE_dirNE
        );
    }
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_3))
    {
        set_tile_facingdir(
            _testMapFull[testTileIndex],
            TileStateDirection::TILE_STATE_dirE
        );
    }
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_4))
    {
        set_tile_facingdir(
            _testMapFull[testTileIndex],
            TileStateDirection::TILE_STATE_dirSE
        );
    }
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_5))
    {
        set_tile_facingdir(
            _testMapFull[testTileIndex],
            TileStateDirection::TILE_STATE_dirS
        );
    }
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_6))
    {
        set_tile_facingdir(
            _testMapFull[testTileIndex],
            TileStateDirection::TILE_STATE_dirSW
        );
    }
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_7))
    {
        set_tile_facingdir(
            _testMapFull[testTileIndex],
            TileStateDirection::TILE_STATE_dirW
        );
    }
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_8))
    {
        set_tile_facingdir(
            _testMapFull[testTileIndex],
            TileStateDirection::TILE_STATE_dirNW
        );
    }
    // test trigger moving
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_E))
    {
        set_tile_action(
            _testMapFull[testTileIndex],
            TileStateAction::TILE_STATE_actionMove
        );
    }
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_R))
    {
        set_tile_action(
            _testMapFull[testTileIndex],
            TileStateAction::TILE_STATE_actionIdle
        );
    }

    // Simulate as if we got updated area from server
    world::WorldObserver& obs = _pWorld->accessObserver();

    int currentTileX = _pWorld->getTileX();
    int currentTileY = _pWorld->getTileY();

    if (s_updateTimer >= s_maxUpdateTimer)
    {
        obs.requestedMapX = currentTileX;
        obs.requestedMapY = currentTileY;

        get_world_state(currentTileX, currentTileY, _observeAreaRadius, _testMapLocal, _testMapFull, _testMapWidth);
        const size_t recvSize = _observeAreaWidth * _observeAreaWidth * sizeof(uint64_t);
        // NOTE: For some fucking reason u have to shift before updating state..
        // -> earlier it was the opposite.
        //  -> this has something to do with the order of operations when testing locally..
        //   -> may fuck up when receiving from server

        obs.lastReceivedMapX = currentTileX;
        obs.lastReceivedMapY = currentTileY;

        _pWorld->shift(obs.lastReceivedMapX, obs.lastReceivedMapY);
        _pWorld->updateObservedArea(_testMapLocal.data());
        _pWorld->moveTerrain();

        //_pWorld->triggerStateUpdate((GC_byte*)_testMapLocal.data(), recvSize);

        s_updateTimer = 0.0f;

    }
    else
    {
        s_updateTimer += 1.0f * Timing::get_delta_time();
    }

    vec3 camPivotPoint = _pCamController->getPivotPoint();
    _pWorld->update(camPivotPoint.x, camPivotPoint.z);

    setInfoText(
        "Delta: " + std::to_string(Timing::get_delta_time())
    );
}

void InGameLocal::lateUpdate()
{
    _pWorld->updateObjects();
}
