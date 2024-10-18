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
}

void InGame::init()
{
    initBase();

    // Set clear color
    Application::get()->getMasterRenderer().setClearColor({ 0, 0, 0, 1});

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
    pClient->addOnMessageEvent(MESSAGE_TYPE__ObjInfoLibResponse, new OnMessagePostLogin_TEST(*this));
    pClient->addOnMessageEvent(MESSAGE_TYPE__LogoutResponse, new OnMessageLogout);

    _pCamController = new CameraController(activeCamera, 16.0f);

    _mousePicker.init((Scene*)this, _pWorld);
    _mousePicker.set3DCursorVisible(true);
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
        _pWorld->updateObjects();
}
