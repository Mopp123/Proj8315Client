#include "InGame.h"
#include "Object.h"
#include "Tile.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "../../Proj8315Common/src/messages/GeneralMessages.h"
#include "MainMenu.h"
#include "core/input/InputEvent.h"
#include "world/Objects.h"


using namespace pk;
using namespace net;

using namespace gamecommon;


void InGame::OnMessageLogin_TEST::onMessage(const GC_byte* data, size_t dataSize)
{
    LoginResponse loginResponse(data, dataSize);

    int32_t x = loginResponse.getTileX();
    int32_t z = loginResponse.getTileZ();

    Client* pClient = Client::get_instance();
    pClient->user.set(
        "", // id
        _sceneRef.testUserName,
        true, // isLoggedIn
        loginResponse.isAdmin(), // isAdmin
        x, // tileX
        z, // tileZ
        "" // factionName
    );

    pClient->send((int32_t)MESSAGE_TYPE__ObjInfoLibRequest, {});
}


void InGame::OnMessagePostLogin_TEST::onMessage(const GC_byte* data, size_t dataSize)
{
    world::objects::ObjectInfoLib::create(data, dataSize);
    Debug::log("___TEST___Obj info lib created");

    _sceneRef.loggedIn = true;
    _sceneRef.createWorld();

    // Set actual camera pos to the received tile coords
    const User& user = Client::get_instance()->user;
    world::World* pWorld = _sceneRef.getWorld();
    float tileVisualScale = pWorld->getTileVisualScale();
    float fx = (float)user.getX() * tileVisualScale;
    float fz = (float)user.getZ() * tileVisualScale;
    Debug::log(
        "___TEST___recv login response! User admin status: " + std::to_string(user.isAdmin()) + " "
        "coords: " + std::to_string(user.getX()) + ", " + std::to_string(user.getZ()) + " "
        "setting visual pos to: " + std::to_string(fx) + ", " + std::to_string(fz) + " "
        "tile scale: " + std::to_string(tileVisualScale)
    );
    _sceneRef.getCamController()->setPivotPoint({ fx, 0, fz });
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

        if (!Panel::is_mouse_over_ui())
        {
            _mousePicker.update(true);
            // TODO: Maybe do this in mouse picker's OnMouseButton event?
            _inGameUI.getSelectedPanel().setSelectedInfo(
                _mousePicker.getSelectedTile(),
                _mousePicker.getSelectedTileX(),
                _mousePicker.getSelectedTileY()
            );
            // TODO: Maybe handle this input stuff somewhere else... maybe using the input events rather than this
            InputManager* pInputManager = Application::get()->accessInputManager();
            if (pInputManager->isMouseButtonDown(InputMouseButtonName::PK_INPUT_MOUSE_RIGHT))
                _inGameUI.getTileOptionsMenu().open(
                    pInputManager->getMouseX(),
                    pInputManager->getMouseY(),
                    _mousePicker.getSelectedTile(),
                    _mousePicker.getSelectedTileX(),
                    _mousePicker.getSelectedTileY()
                );
        }

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
