#include "InGameUI.h"
#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/Tile.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "Object.h"
#include "world/Objects.h"
#include "scenes/InGame.h"
#include "scenes/MainMenu.h"


using namespace pk;
using namespace pk::ui;
using namespace world;
using namespace net;

using namespace gamecommon;


void InGameUI::OnClickLogout::onClick(pk::InputMouseButtonName button)
{
    Client* pClient = Client::get_instance();

    if (pClient->isConnected())
    {
        pClient->send((int32_t)MESSAGE_TYPE__LogoutRequest, {});
        pScene->loggingOut = true;
        pScene->setInfoText(
            "Logging out...",
            { 1, 1, 0 },
            0, 0,
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL
        );
    }
}


void InGameUI::OnMessageLogout::onMessage(const GC_byte* data, size_t dataSize)
{
    Client::get_instance()->user = NULL_USER;
    Application::get()->switchScene(new MainMenu);
}


// TODO:
// * Some func to add and store status and attribute strings/values more clearly
// * Display "really" selected object's info
void InGameUI::create(
    BaseScene* pScene,
    world::World* pWorld,
    CameraController* pCamController,
    pk::Font* pFont,
    pk::Font* pSmallFont
)
{
    _pScene = pScene;

    const vec2 settingsPanelScale(212, 30);
    const vec2 settingsPanelSlotScale(100, 24);
    _settingsPanel.createDefault(
        _pScene,
        pFont,
        {
            HorizontalConstraintType::PIXEL_RIGHT, -settingsPanelScale.x,
            VerticalConstraintType::PIXEL_TOP, 0.0f,
        },
        settingsPanelScale,
        settingsPanelSlotScale,
        Panel::LayoutFillType::HORIZONTAL
    );
    _settingsPanel.addDefaultButton(
        "Logout",
        new OnClickLogout(_pScene),
        100
    );

    _settingsPanel.addDefaultButton(
        "Settings",
        nullptr,
        100
    );

    Client* pClient = Client::get_instance();
    if (pClient)
        pClient->addOnMessageEvent(MESSAGE_TYPE__LogoutResponse, new OnMessageLogout);

    _selectedPanel.init(pScene, pSmallFont);
    _tileOptionsMenu.init(pScene, pWorld, pCamController, pSmallFont);
}
