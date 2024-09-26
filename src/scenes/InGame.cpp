#include "InGame.h"
#include "Tile.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "MainMenu.h"


using namespace pk;
using namespace net;


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
        gamecommon::set_tile_terrelevation(_testMapFull[i + 0 * _testMapWidth], 5);
    for (int i = 0; i < _testMapWidth; ++i)
        gamecommon::set_tile_terrelevation(_testMapFull[0 + i * _testMapWidth], 5);

    gamecommon::set_tile_terrtype(_testMapFull[2 + 2 * _testMapWidth], 1);

    _testMapLocal.resize(_observeAreaWidth * _observeAreaWidth * sizeof(uint64_t), 0);

    _pWorld->updateObservedArea(_testMapLocal.data());
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

    // Simulate as if we got updated area from server
    /*
    world::WorldObserver& obs = _pWorld->accessObserver();
    int reqX = obs.requestedMapX;
    int reqY = obs.requestedMapY;

    int gridX = obs.lastReceivedMapX;
    int gridY = obs.lastReceivedMapY;

    if (gridX != reqX || gridY != reqY)
    {
        get_world_state(gridX, gridY, _observeAreaRadius, _testMapLocal, _testMapFull, _testMapWidth);
        _pWorld->setAreaState(_testMapLocal);
        obs.lastReceivedMapX = reqX;
        obs.lastReceivedMapY = reqY;
        Debug::log("___TEST___received from pos: " + std::to_string(gridX) + ", " + std::to_string(gridY));
    }
    */

    Transform* pCamTransform = (Transform*)getComponent(activeCamera, ComponentType::PK_TRANSFORM);
    mat4& camTMat = pCamTransform->accessTransformationMatrix();
    _pWorld->update(camTMat[0 + 3 * 4], camTMat[2 + 3 * 4]);

    if (loggedIn && !loggingOut)
        setInfoText(
            "Delta: " + std::to_string(Timing::get_delta_time())
        );
}

void InGame::lateUpdate()
{
}
