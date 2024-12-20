#include "InGameLocal.h"
#include "Object.h"
#include "Tile.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "MainMenu.h"
#include "world/Objects.h"

#include "pesukarhu/ppk.h"


using namespace pk;
using namespace pk::ui;
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


static void create_ramp(
    uint64_t* pWorld,
    int worldWidth,
    int posX,
    int posY,
    int rampWidth
)
{
    GC_ubyte rampBeginHeight = get_tile_terrelevation(*(pWorld + (posX + posY * worldWidth)));
    int jCount = rampBeginHeight;
    int iCount = rampWidth;

    TileStateDirection direction = TileStateDirection::TILE_STATE_dirN;

    // Figure out which direction ramp should descent
    int rampBuildThreshold = 2;
    int dirWeights[8];
    memset(dirWeights, 0, sizeof(int) * 8);
    // Also keep track of the adjacent tile heights to figure out what our target height should be
    // after direction has been figured
    GC_ubyte dirHeights[8];
    memset(dirHeights, 0, 8);
    int dirHeightsIndex = 0;
    for (int y = posY - 1; y <= posY + 1; ++y)
    {
        for (int x = posX - 1; x <= posX + 1; ++x)
        {
            if (x < 0 || x >= worldWidth || y < 0 || y >= worldWidth || (x == posX && y == posY))
                continue;

            GC_ubyte adjacentElevation = get_tile_terrelevation(*(pWorld + (x + y * worldWidth)));

            dirHeights[dirHeightsIndex] = adjacentElevation;
            ++dirHeightsIndex;

            if ((int)rampBeginHeight -  (int)adjacentElevation >= rampBuildThreshold)
            {
                // north weight
                if (y == posY - 1)
                    dirWeights[TileStateDirection::TILE_STATE_dirN] += 1 +dirWeights[TileStateDirection::TILE_STATE_dirN];
                // south weight
                else if (y == posY + 1)
                    dirWeights[TileStateDirection::TILE_STATE_dirS] += 1 + dirWeights[TileStateDirection::TILE_STATE_dirS];

                // east weight
                if (x == posX + 1)
                    dirWeights[TileStateDirection::TILE_STATE_dirE] += 1 + dirWeights[TileStateDirection::TILE_STATE_dirE];
                // west weight
                if (x == posX - 1)
                    dirWeights[TileStateDirection::TILE_STATE_dirW] += 1 + dirWeights[TileStateDirection::TILE_STATE_dirW];
            }
        }
    }
    // sort dirHeights to mach direction indexing
    GC_ubyte swapTarget[8] = { 1, 2, 4, 7, 6, 5, 3, 0 };
    GC_ubyte sortedDirHeights[8];
    for (int i = 0; i < 8; ++i)
        sortedDirHeights[i] = dirHeights[swapTarget[i]];

    // calc diagonal dir weights
    dirWeights[TileStateDirection::TILE_STATE_dirNE] = (dirWeights[TileStateDirection::TILE_STATE_dirN] + dirWeights[TileStateDirection::TILE_STATE_dirE]) / 2;
    dirWeights[TileStateDirection::TILE_STATE_dirSE] = (dirWeights[TileStateDirection::TILE_STATE_dirS] + dirWeights[TileStateDirection::TILE_STATE_dirE]) / 2;
    dirWeights[TileStateDirection::TILE_STATE_dirSW] = (dirWeights[TileStateDirection::TILE_STATE_dirS] + dirWeights[TileStateDirection::TILE_STATE_dirW]) / 2;
    dirWeights[TileStateDirection::TILE_STATE_dirNW] = (dirWeights[TileStateDirection::TILE_STATE_dirN] + dirWeights[TileStateDirection::TILE_STATE_dirW]) / 2;
    // Prioritize diagonal by adding 1 to non zeros
    for (int i = 1; i <= 7; i += 2)
        dirWeights[i] = dirWeights[i] > 0 ? dirWeights[i] + 1 : dirWeights[i];

    int highestVal = 0;
    int highestDir = 0;
    for (int i = 0; i < 8; ++i)
    {
        int val = dirWeights[i];
        if (val > highestVal)
        {
            highestVal = val;
            highestDir = i;
        }
    }
    direction = (TileStateDirection)highestDir;

    // Figure out what the target height should be
    GC_ubyte targetHeight = sortedDirHeights[highestDir];

    bool diagonal = direction == TileStateDirection::TILE_STATE_dirNE ||
        direction == TileStateDirection::TILE_STATE_dirSE ||
        direction == TileStateDirection::TILE_STATE_dirSW ||
        direction == TileStateDirection::TILE_STATE_dirNW;

    // Need to make a bit wider if diagonal..
    if (diagonal)
    {
        iCount += 1;
        rampWidth += 1;
    }

    // Center the ramp little better in following cases.. looks better..
    if (direction == TileStateDirection::TILE_STATE_dirN || direction == TileStateDirection::TILE_STATE_dirS)
        posX -= rampWidth / 2;
    if (direction == TileStateDirection::TILE_STATE_dirE || direction == TileStateDirection::TILE_STATE_dirW)
        posY -= rampWidth / 2;

    for (int j = 0; j < jCount; ++j)
    {
        for (int i = 0; i < iCount; ++i)
        {
            int tileY = posY + j;
            int tileX = posX + i;
            int steepnessModifier = j * 2;

            if (direction == TileStateDirection::TILE_STATE_dirN)
            {
                tileY = posY - j;
            }
            else if (direction == TileStateDirection::TILE_STATE_dirW)
            {
                tileY = posY + i;
                tileX = posX - j;
            }
            else if (direction == TileStateDirection::TILE_STATE_dirE)
            {
                tileY = posY + i;
                tileX = posX + j;
            }
            // Diagonal cases are a bit.. special..
            else if (direction == TileStateDirection::TILE_STATE_dirNE)
            {
                tileY = posY - j;
                tileX = posX + (i + j) - (std::ceil(rampWidth / 2));
                steepnessModifier = j * 2 + i;
            }
            else if (direction == TileStateDirection::TILE_STATE_dirSE)
            {
                tileY = posY + j;
                tileX = posX + (i + j) - (std::ceil(rampWidth / 2));
                steepnessModifier = j * 2 + i;
            }
            else if (direction == TileStateDirection::TILE_STATE_dirSW)
            {
                tileY = posY + j;
                tileX = posX - ((rampWidth - 1) - i) - j + (std::ceil(rampWidth / 2));
                steepnessModifier = j * 2 + ((rampWidth - 1) - i);
            }
            else if (direction == TileStateDirection::TILE_STATE_dirNW)
            {
                tileY = posY - j;
                tileX = posX - (i + j) + (std::ceil(rampWidth / 2));
                steepnessModifier = j * 2 + i;
            }

            if (tileX < 0 || tileX >= worldWidth || tileY < 0 || tileY >= worldWidth)
            {
                continue;
            }

            int finalHeight = (int)rampBeginHeight - steepnessModifier;

            if (finalHeight >= targetHeight)
            {
                const GC_ubyte h = (GC_ubyte)finalHeight;
                uint64_t& targetTile = *(pWorld + (tileX + tileY * worldWidth));
                set_tile_terrelevation(targetTile, h);
                set_tile_terrtype(targetTile, 4);
            }
        }
    }
}


static void test_area(
    std::vector<uint64_t>& mapData,
    int mapWidth,
    int startX,
    int startY,
    TileStateTerrType backgroundType,
    TileStateTemperature backgroundTemp,
    TileStateTerrType foregroundType,
    TileStateTemperature foregroundTemp
)
{
    const int width = 5;
    for (int y = 0; y < width; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int xPos = startX + x;
            int yPos = startY + y;
            if (x < 2 && y < 2)
            {
                set_tile_terrtype(mapData[xPos + yPos * mapWidth], foregroundType);
                set_tile_temperature(mapData[xPos + yPos * mapWidth], foregroundTemp);
            }
            else
            {
                set_tile_terrtype(mapData[xPos + yPos * mapWidth], backgroundType);
                set_tile_temperature(mapData[xPos + yPos * mapWidth], backgroundTemp);
            }
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

    _pCamController = new CameraController(activeCamera, 30.0f);
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
    uint64_t treeObjState = 0;
    set_tile_thingid(treeObjState, 1);
    gamecommon::ObjectInfo treeObj(
        "Tree",
        "Doesn't do anything",
        {
            "Idle"
        },
        0,
        treeObjState
    );
    uint64_t spruceTreeObjState = 0;
    set_tile_thingid(spruceTreeObjState, 2);
    gamecommon::ObjectInfo spruceTreeObj(
        "Spruce Tree",
        "Doesn't do anything2",
        {
            "Idle"
        },
        0,
        spruceTreeObjState
    );
    uint64_t palmTreeObjState = 0;
    set_tile_thingid(palmTreeObjState, 3);
    gamecommon::ObjectInfo palmTreeObj(
        "Palm Tree",
        "Doesn't do anything2",
        {
            "Idle"
        },
        0,
        palmTreeObjState
    );
    uint64_t movingObjState = 0;
    set_tile_thingid(movingObjState, 4);
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
            treeObj,
            spruceTreeObj,
            palmTreeObj,
            movingObj
        }
    );
    Debug::log("___TEST___created obj info lib:\n" + world::objects::ObjectInfoLib::to_string());

    world::objects::ObjectInfoLib::load_object_visuals();

    // World can be created only after object info lib creation
    _pWorld = new world::World(
        (Scene&)(*this),
        pCamTransform,
        _observeAreaRadius,
        4.0f
    );
    _mousePicker.init((Scene*)this, _pWorld);

    _inGameUI.create(this, _pWorld, _pCamController, _pDefaultFont, _pSmallFont);


    uint64_t initialTileState = 0;
    set_tile_terrtype(initialTileState, TileStateTerrType::TILE_STATE_terrTypeDirt);
    _testMapFull.resize(_testMapWidth * _testMapWidth * sizeof(uint64_t), initialTileState);

    set_tile_thingid(_testMapFull[5 + 5 * _testMapWidth], 4);
    set_tile_thingid(_testMapFull[6 + 7 * _testMapWidth], 1);
    set_tile_thingid(_testMapFull[5 + 8 * _testMapWidth], 2);
    set_tile_thingid(_testMapFull[6 + 8 * _testMapWidth], 3);

    const int testAreaWidth = 5;
    test_area(
        _testMapFull,
        _testMapWidth,
        0,
        0,
        TileStateTerrType::TILE_STATE_terrTypeDirt,
        TileStateTemperature::TILE_STATE_mild,
        TileStateTerrType::TILE_STATE_terrTypeDunes,
        TileStateTemperature::TILE_STATE_freezing
    );

    test_area(
        _testMapFull,
        _testMapWidth,
        15,
        15,
        TileStateTerrType::TILE_STATE_terrTypeDirt,
        TileStateTemperature::TILE_STATE_mild,
        TileStateTerrType::TILE_STATE_terrTypeDunes,
        TileStateTemperature::TILE_STATE_burning
    );

    _testMapLocal.resize(_observeAreaWidth * _observeAreaWidth * sizeof(uint64_t), 0);
}


static float s_updateTimer = 0.0f;
static float s_maxUpdateTimer = 0.5f;
static int s_TEST_clickState = 0;
static int s_TEST_keyStateJ = 0;
static int s_TEST_keyStateK = 0;

void InGameLocal::update()
{
    _pCamController->update();

    InputManager* pInputManager = Application::get()->accessInputManager();

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
        _pWorld->updateObjects();

        //_pWorld->triggerStateUpdate((GC_byte*)_testMapLocal.data(), recvSize);

        s_updateTimer = 0.0f;
    }
    else
    {
        s_updateTimer += 1.0f * Timing::get_delta_time();
    }

    vec3 camPivotPoint = _pCamController->getPivotPoint();



    // Test change selected obj info
    // TODO: Maybe do this in mouse picker's OnMouseButton event?
    if (!Panel::is_mouse_over_ui())
    {
        _mousePicker.update(true);
        _inGameUI.getSelectedPanel().setSelectedInfo(
            _mousePicker.getSelectedTile(),
            _mousePicker.getSelectedTileX(),
            _mousePicker.getSelectedTileY()
        );
        if (pInputManager->isMouseButtonDown(InputMouseButtonName::PK_INPUT_MOUSE_RIGHT))
            _inGameUI.getTileOptionsMenu().open(
                pInputManager->getMouseX(),
                pInputManager->getMouseY(),
                _mousePicker.getSelectedTile(),
                _mousePicker.getSelectedTileX(),
                _mousePicker.getSelectedTileY()
            );
    }


    // Test put ramp on mouse click
    if (pInputManager->isMouseButtonDown(InputMouseButtonName::PK_INPUT_MOUSE_RIGHT))
        s_TEST_clickState += 1;
    else
        s_TEST_clickState = 0;

    int pickedTileX = _mousePicker.getPickedTileX();
    int pickedTileY = _mousePicker.getPickedTileY();
    if (s_TEST_clickState == 1)
    {
        create_ramp(
            _testMapFull.data(),
            _testMapWidth,
            pickedTileX,
            pickedTileY,
            3
        );
    }
    // Test incr height with "k" and decr with "j" keys
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_J))
        s_TEST_keyStateJ += 1;
    else
        s_TEST_keyStateJ = 0;

    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_K))
        s_TEST_keyStateK += 1;
    else
        s_TEST_keyStateK = 0;

    if (s_TEST_keyStateJ == 1 || s_TEST_keyStateK == 1)
    {
        int radius = 3;
        int elevationIncr = 4;
        if (s_TEST_keyStateJ)
            elevationIncr = -4;

        int currentHeight = (int)get_tile_terrelevation(_testMapFull[pickedTileX + pickedTileY * _testMapWidth]);
        for (int y = pickedTileY - radius; y <= pickedTileY + radius; ++y)
        {
            for (int x = pickedTileX - radius; x <= pickedTileX + radius; ++x)
            {
                int tileIndex = x + y * _testMapWidth;
                if (tileIndex >= 0 && tileIndex < (_testMapWidth * _testMapWidth))
                {
                    int newHeight = currentHeight + elevationIncr;
                    if (newHeight >= 0 && newHeight <= 31)
                    {
                        set_tile_terrelevation(_testMapFull[x + y * _testMapWidth], currentHeight + elevationIncr);
                        set_tile_terrtype(_testMapFull[x + y * _testMapWidth], 4);
                    }
                }
            }
        }
    }


    _pWorld->update(camPivotPoint.x, camPivotPoint.z);


    setInfoText(
        "Delta: " + std::to_string(Timing::get_delta_time())
    );
}

void InGameLocal::lateUpdate()
{
    _pWorld->updateObjects();
}
