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

    _pWorld = new world::World(
        (Scene&)(*this),
        pCamTransform,
        _observeAreaRadius
    );

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

    set_tile_terrelevation(_testMapFull[2 + 2 * _testMapWidth], 2);
    set_tile_terrelevation(_testMapFull[3 + 2 * _testMapWidth], 2);
    set_tile_terrelevation(_testMapFull[2 + 3 * _testMapWidth], 2);
    set_tile_terrelevation(_testMapFull[3 + 3 * _testMapWidth], 2);

    _testMapLocal.resize(_observeAreaWidth * _observeAreaWidth * sizeof(uint64_t), 0);

}


static float s_updateTimer = 0.0f;
static float s_maxUpdateTimer = 0.5f;

void InGameLocal::update()
{
    _pCamController->update();

    InputManager* pInputManager = Application::get()->accessInputManager();
    // test update tile dir
    int testTileIndex = 3 + 3 * _testMapWidth;
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
    int reqX = obs.requestedMapX;
    int reqY = obs.requestedMapY;

    int gridX = obs.lastReceivedMapX;
    int gridY = obs.lastReceivedMapY;

    if (s_updateTimer >= s_maxUpdateTimer)
    {
        get_world_state(gridX, gridY, _observeAreaRadius, _testMapLocal, _testMapFull, _testMapWidth);
        const size_t recvSize = _observeAreaWidth * _observeAreaWidth * sizeof(uint64_t);
        // NOTE: For some fucking reason u have to shift before updating state..
        // -> earlier it was the opposite.
        //  -> this has something to do with the order of operations when testing locally..
        //   -> may fuck up when receiving from server
        _pWorld->shift(obs.lastReceivedMapX, obs.lastReceivedMapY);
        _pWorld->updateObservedArea(_testMapLocal.data());

        //_pWorld->triggerStateUpdate((GC_byte*)_testMapLocal.data(), recvSize);

        s_updateTimer = 0.0f;

        obs.lastReceivedMapX = reqX;
        obs.lastReceivedMapY = reqY;
    }
    else
    {
        s_updateTimer += 1.0f * Timing::get_delta_time();
    }

    Transform* pCamTransform = (Transform*)getComponent(activeCamera, ComponentType::PK_TRANSFORM);
    mat4& camTMat = pCamTransform->accessTransformationMatrix();

    _pWorld->update(camTMat[0 + 3 * 4], camTMat[2 + 3 * 4]);

    // Test glue cam to ground
    camTMat[1 + 3 * 4] = _pWorld->getTerrainHeight(camTMat[0 + 3 * 4], camTMat[2 + 3 * 4]) + 1.0f;

    setInfoText(
        "Delta: " + std::to_string(Timing::get_delta_time())
    );
}
