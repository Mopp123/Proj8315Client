#include "InGame.h"
#include "Object.h"
#include "Tile.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "MainMenu.h"
#include "core/input/InputEvent.h"
#include "world/Objects.h"


using namespace pk;
using namespace net;

using namespace gamecommon;


void InGame::OnMessageLogin_TEST::onMessage(const GC_byte* data, size_t dataSize)
{
    Debug::log("___TEST___recv login response");
    Client* pClient = Client::get_instance();
    pClient->send((int32_t)MESSAGE_TYPE__ObjInfoLibRequest, {});
}


void InGame::OnMessagePostLogin_TEST::onMessage(const GC_byte* data, size_t dataSize)
{
    world::objects::ObjectInfoLib::create(data, dataSize);
    Debug::log("___TEST___Obj info lib created");

    Client* pClient = Client::get_instance();
    _sceneRef.loggedIn = true;
    pClient->user.name = _sceneRef.testUserName;
    pClient->user.isLoggedIn = true;
    _sceneRef.createWorld();
}


InGame::InGame()
{}

InGame::~InGame()
{
    delete _pCamController;
}

void InGame::createWorld()
{
    Debug::log("___TEST___Creating world...");
    Transform* pCamTransform = (Transform*)getComponent(activeCamera, ComponentType::PK_TRANSFORM);

    _pWorld = new world::World(
        (Scene&)(*this),
        pCamTransform,
        _observeAreaRadius,
        4.0f
    );
    Debug::log("___TEST___Creating world -> SUCCESS");
    // Can create mousepicker only when the "world" is available
    _mousePicker.init((Scene*)this, _pWorld);

    // Just to be safe create/init inGameUI here as well..
    _inGameUI.create(this, (Scene*)this, _pDefaultFont, _pSmallFont);
}

void InGame::init()
{
    initBase();

    // Set clear color
    Application::get()->getMasterRenderer().setClearColor({ 0, 0, 0, 1});

    Client* pClient = Client::get_instance();
    pClient->addOnMessageEvent(MESSAGE_TYPE__LoginResponse, new OnMessageLogin_TEST(*this));
    pClient->addOnMessageEvent(MESSAGE_TYPE__ObjInfoLibResponse, new OnMessagePostLogin_TEST(*this));

    _pCamController = new CameraController(activeCamera, 32.0f);
    // test setting cam pos initially to "server side equator" (worldPos * tileVisualScale)
    _pCamController->setPivotPoint({ 128, 0, 4.0f * 128.0f });
}

void InGame::update()
{
    // Attempt logging in some test user immediately
    if (!loggedIn && !waitingLogin)
    {
        Client* pClient = Client::get_instance();
        if(pClient->isConnected())
        {
            Debug::log("___TEST___sending login request");
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

    if (_pWorld)
    {
        _pCamController->update();

        vec3 camPivotPoint = _pCamController->getPivotPoint();
        _pCamController->setPivotPointHeight(_pWorld->getTerrainHeight(camPivotPoint.x, camPivotPoint.z));

        _mousePicker.update(true);
        // TODO: Maybe do this in mouse picker's OnMouseButton event?
        _inGameUI.getSelectedPanel().setSelectedInfo(
            _mousePicker.getSelectedTile(),
            _mousePicker.getSelectedTileX(),
            _mousePicker.getSelectedTileY()
        );

        _pWorld->update(camPivotPoint.x, camPivotPoint.z);

        if (loggedIn && !loggingOut)
            setInfoText(
                "Delta: " + std::to_string(Timing::get_delta_time())
            );
    }
}

void InGame::lateUpdate()
{
    if (_pWorld)
    {
        _pWorld->updateObjects();
    }
}
