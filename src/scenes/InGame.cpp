#include "InGame.h"
#include "Tile.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "MainMenu.h"
#include "core/input/InputEvent.h"


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


void InGame::OnClickLogout::onClick(pk::InputMouseButtonName button)
{
    Client::get_instance()->send((int32_t)MESSAGE_TYPE__LogoutRequest, {});
    _sceneRef.loggingOut = true;
    _sceneRef.setInfoText(
        "Logging out...",
        { 1, 1, 0 },
        0, 0,
        HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
        VerticalConstraintType::PIXEL_CENTER_VERTICAL
    );
}


void InGame::OnMessageLogin_TEST::onMessage(const GC_byte* data, size_t dataSize)
{
    _sceneRef.loggedIn = true;
    Client* pClient = Client::get_instance();
    pClient->user.name = _sceneRef.testUserName;
    pClient->user.isLoggedIn = true;
}


void InGame::OnMessageLogout::onMessage(const GC_byte* data, size_t dataSize)
{
    Client::get_instance()->user.isLoggedIn = false;
    Application::get()->switchScene(new MainMenu);
}


InGame::InGame()
{}

InGame::~InGame()
{
    delete _pCamController;
}

void InGame::init()
{
    initBase();

    _mainPanel.createDefault(
        (Scene*)this,
        _pDefaultFont,
        HorizontalConstraintType::PIXEL_RIGHT, 5,
        VerticalConstraintType::PIXEL_TOP, 2,
        { 120, 25 },
        Panel::LayoutFillType::HORIZONTAL
    );
    _mainPanel.addDefaultButton(
        "Logout",
        new OnClickLogout(*this),
        100
    );

    Client* pClient = Client::get_instance();
    pClient->addOnMessageEvent(MESSAGE_TYPE__LoginResponse, new OnMessageLogin_TEST(*this));
    pClient->addOnMessageEvent(MESSAGE_TYPE__LogoutResponse, new OnMessageLogout);

    _pCamController = new CameraController(activeCamera, 10.0f);

    Transform* pCamTransform = (Transform*)getComponent(activeCamera, ComponentType::PK_TRANSFORM);

    _pWorld = new world::World(
        (Scene&)(*this),
        pCamTransform,
        _observeAreaRadius
    );

    _testMapFull.resize(_testMapWidth * _testMapWidth * sizeof(uint64_t), 0);
    for (int i = 0; i < _testMapWidth; ++i)
        set_tile_terrelevation(_testMapFull[i + 0 * _testMapWidth], 5);
    for (int i = 0; i < _testMapWidth; ++i)
        set_tile_terrelevation(_testMapFull[0 + i * _testMapWidth], 5);

    set_tile_terrtype(_testMapFull[2 + 2 * _testMapWidth], 1);

    set_tile_terrelevation(_testMapFull[3 + 3 * _testMapWidth], 3);
    set_tile_thingid(_testMapFull[3 + 3 * _testMapWidth], 1);
    set_tile_facingdir(_testMapFull[3 + 3 * _testMapWidth], 3);

    _testMapLocal.resize(_observeAreaWidth * _observeAreaWidth * sizeof(uint64_t), 0);
}

void InGame::update()
{
    // Attempt logging in some test user immediately
    if (!loggedIn && !waitingLogin)
    {
        Client* pClient = Client::get_instance();
        if(pClient->isConnected())
        {
            waitingLogin = true;
            pClient->send(
                (int32_t)MESSAGE_TYPE__LoginRequest,
                {
                    {
                        (GC_byte*)testUserName.data(),
                        testUserName.size(),
                        USER_NAME_SIZE
                    },
                    {
                        (GC_byte*)testUserPassword.data(),
                        testUserPassword.size(),
                        USER_PASSWD_SIZE
                    }
                }
            );
            setInfoText(
                "Logging in...",
                { 1, 1, 0 },
                0, 0,
                HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
                VerticalConstraintType::PIXEL_CENTER_VERTICAL
            );
        }
        else
        {
            setInfoText(
                "Connecting...",
                { 1, 1, 0 },
                0, 0,
                HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
                VerticalConstraintType::PIXEL_CENTER_VERTICAL
            );
        }
    }

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
        Debug::log("___TEST___dir set to N");
    }
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_2))
    {
        set_tile_facingdir(
            _testMapFull[testTileIndex],
            TileStateDirection::TILE_STATE_dirNE
        );
        Debug::log("___TEST___dir set to NE");
    }
    if (pInputManager->isKeyDown(InputKeyName::PK_INPUT_KEY_3))
    {
        set_tile_facingdir(
            _testMapFull[testTileIndex],
            TileStateDirection::TILE_STATE_dirE
        );
        Debug::log("___TEST___dir set to E");
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

    // Simulate as if we got updated area from server
    world::WorldObserver& obs = _pWorld->accessObserver();
    int reqX = obs.requestedMapX;
    int reqY = obs.requestedMapY;

    int gridX = obs.lastReceivedMapX;
    int gridY = obs.lastReceivedMapY;

    //if (gridX != reqX || gridY != reqY)
    //{
        get_world_state(gridX, gridY, _observeAreaRadius, _testMapLocal, _testMapFull, _testMapWidth);
        const size_t recvSize = _observeAreaWidth * _observeAreaWidth * sizeof(uint64_t);
        _pWorld->triggerStateUpdate((const GC_byte*)_testMapLocal.data(), recvSize);
    //}

    Transform* pCamTransform = (Transform*)getComponent(activeCamera, ComponentType::PK_TRANSFORM);
    mat4& camTMat = pCamTransform->accessTransformationMatrix();
    _pWorld->update(camTMat[0 + 3 * 4], camTMat[2 + 3 * 4]);

    obs.lastReceivedMapX = reqX;
    obs.lastReceivedMapY = reqY;



    if (loggedIn && !loggingOut)
        setInfoText(
            "Delta: " + std::to_string(Timing::get_delta_time())
        );
}

void InGame::lateUpdate()
{
}
