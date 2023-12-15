#include "MainMenu.h"
#include "../net/Client.h"
#include "LoginMenu.h"
#include "DeploymentMenu.h"
#include "../../Proj8315Common/src/Faction.h"
#include "../../Proj8315Common/src/messages/Message.h"


using namespace pk;
using namespace web;
using namespace ui;
using namespace net;
using namespace gamecommon;


void MainMenu::OnClickCreateFaction::onClick(InputMouseButtonName button)
{
    if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
    {
        Debug::log("___TEST___sending create faction");
        std::string requestedFactionName = pCreateFactionInputFieldTxt->getStr();
        Faction newFaction(requestedFactionName.data(), requestedFactionName.size());
        const size_t factionNetwSize = Faction::get_netw_size();
        Client::get_instance()->send(
            (uint32_t)MESSAGE_TYPE__CreateFaction,
            {
                {
                    (PK_byte*)newFaction.getNetwData(),
                    factionNetwSize,
                    factionNetwSize
                }
            }
        );
    }
}


void MainMenu::OnClickDeploymentMenu::onClick(InputMouseButtonName button)
{
    if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
    {
        Application::get()->switchScene((Scene*)new DeploymentMenu());
    }
}


// TODO: Send the actual logout message
void MainMenu::OnClickLogout::onClick(InputMouseButtonName button)
{
    if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
    {
        Application::get()->switchScene((Scene*)(new LoginMenu));
    }
}


void MainMenu::OnMessageMOTD::onMessage(const PK_byte* data, size_t dataSize)
{
    Debug::log("___TEST___onmessage MOTD! (WARNING: NOT WORKING PROPERLY ATM!)");
    size_t msgSize = dataSize;
    if (msgSize > 256)
        msgSize = 256;
    PK_byte* buf = new PK_byte[msgSize];
    memcpy(buf, data, msgSize);
    pMOTDTxt->accessStr() = std::string(buf, msgSize);
    delete[] buf;
}


// TODO: Create Faction response message
void MainMenu::OnMessageCreateFaction::onMessage(const PK_byte* data, size_t dataSize)
{
    Debug::log("___TEST___onmessage create faction");
    Client* client = Client::get_instance();
    bool success = *((bool*)data);
    if (success)
    {
        if (dataSize > 1)
        {
            Faction faction(data + 1, FACTION_NAME_SIZE);
            client->user.faction = faction.getName();
            client->user.hasFaction = true;
            Debug::log("Faction creation was successful");
            sceneRef.showFactionMenu(true);
            ((BaseScene&)sceneRef).setInfoText("");
        }
    }
    else
    {
        const size_t errMessagePos = 1 + Faction::get_netw_size();
        const size_t errMessageSize = dataSize - errMessagePos;
        if (dataSize > errMessagePos)
        {
            char infoMessage[MESSAGE_INFO_MESSAGE_LEN];
            memset(infoMessage, 0, MESSAGE_INFO_MESSAGE_LEN);
            memcpy(infoMessage, data + errMessagePos, errMessageSize);
            ((BaseScene&)sceneRef).setInfoText(std::string(infoMessage));
        }
        else
        {
            Debug::log("Failed to create faction but server didn't provide error/info message", Debug::MessageType::PK_WARNING);
        }
    }
}


MainMenu::MainMenu()
{}

MainMenu::~MainMenu()
{
    Client::get_instance()->clearOnMessageEvents();
}


void MainMenu::init()
{
    ((BaseScene*)this)->initBase();

    vec2 originPos_infoPanel{100, 50};

    vec2 originPos_mainFuncPanel{100, 120};
    vec2 buttonScale_mainFuncPanel{100, 24};

    // Info panel
    _pMOTDTxt = create_text(
        "Server message of the day:\nNone",
        ConstraintType::PIXEL_LEFT, originPos_infoPanel.x,
        ConstraintType::PIXEL_TOP, originPos_infoPanel.y
    ).second;


    // Main func panel
    _mainFuncPanel.init(
        { ConstraintType::PIXEL_CENTER_HORIZONTAL, -250},
        { ConstraintType::PIXEL_CENTER_VERTICAL, 180},
        { 130, 24 }
    );
    _mainFuncPanel.addButton("Enter Planet", new OnClickDeploymentMenu);
    _mainFuncPanel.addButton("Options", nullptr);
    _mainFuncPanel.addButton("Logout", new OnClickLogout);

    _mainFuncPanel.setActive(false);

    // Create faction panel (displayed on login only if no user faction data was found)
    _createFactionPanel.init(
        { ConstraintType::PIXEL_CENTER_HORIZONTAL, -150 },
        { ConstraintType::PIXEL_CENTER_VERTICAL, 32 },
        { 130, 24 },
        true, { 300, 140 }
    );
    _createFactionPanel.addText("Name your faction");
    TextRenderable* inputFieldContent = _createFactionPanel.addInputField("", 292, nullptr).second;
    _createFactionPanel.addButton("Create", new OnClickCreateFaction(inputFieldContent));
    // Currently used for displaying error/info messages from server
    _pInfoTxt = create_text(
        "",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, -150,
        ConstraintType::PIXEL_CENTER_VERTICAL, 120
    ).second;

    _pFactionInfoTxt = create_text(
        "TestFactionNameHere123",
        ConstraintType::PIXEL_CENTER_HORIZONTAL, 120,
        ConstraintType::PIXEL_CENTER_VERTICAL, 180
    ).second;

    Client* client = Client::get_instance();

    showFactionMenu(client->user.hasFaction);

    client->addOnMessageEvent(
        MESSAGE_TYPE__ServerMessage,
        (OnMessageEvent*)(new OnMessageMOTD(_pMOTDTxt))
    );
    client->addOnMessageEvent(
        MESSAGE_TYPE__CreateFaction,
        (OnMessageEvent*)(new OnMessageCreateFaction(*this))
    );
    client->send(MESSAGE_TYPE__ServerMessage, {});
}

void MainMenu::update()
{}

void MainMenu::showFactionMenu(bool arg)
{
    _createFactionPanel.setActive(!arg);
    _mainFuncPanel.setActive(arg);

    _pFactionInfoTxt->setActive(arg);
}

