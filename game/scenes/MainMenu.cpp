#include "MainMenu.h"
#include "../net/Client.h"
#include "../net/NetCommon.h"
#include "LoginMenu.h"
#include "DeploymentMenu.h"


using namespace pk;
using namespace web;
using namespace ui;
using namespace net;


FactionBuilding::FactionBuilding(
    std::string type, 
    pk::vec3 pos, 
    pk::vec2 scale, 
    pk::Scene* scene, 
    pk::Texture* spriteSheetTexture,
    vec2 texOffset
) :
    _type(type)
{
    _entity = scene->createEntity();
    _pRenderable = new Sprite3DRenderable(
        pos, scale, 
        spriteSheetTexture
    );
    _pRenderable->textureOffset = texOffset;
    scene->addComponent(_entity, (Component*)_pRenderable);
}

// *just copying here all including prts as well, since this doesn't own any of this
FactionBuilding::FactionBuilding(const FactionBuilding& other)
{
    _entity = other._entity;
    _pRenderable = other._pRenderable;
    _type = other._type;
}


class OnMessageMOTD : public net::OnMessageEvent
{
public:
    TextRenderable* pMOTDTxt = nullptr;

    OnMessageMOTD(TextRenderable* pMOTD) : 
        pMOTDTxt(pMOTD)
    {}

    virtual void onMessage(const PK_byte* data, size_t dataSize)
    {
        size_t msgSize = dataSize;
        if (msgSize > 256)
            msgSize = 256;
        PK_byte* buf = new PK_byte[msgSize];
        memcpy(buf, data, msgSize);
        pMOTDTxt->accessStr() = std::string(buf, msgSize);
        delete[] buf;
    }
};


class OnMessageFactionInfo : public net::OnMessageEvent
{
public:
    OnMessageFactionInfo()
    {}

    virtual void onMessage(const PK_byte* data, size_t dataSize)
    {

    }
};


class OnClickCreateFaction : public OnClickEvent
{
public:
    MainMenu& sceneRef;

    OnClickCreateFaction(MainMenu& sceneRef) :
        sceneRef(sceneRef)
    {}

    // TODO: Send the actual faction creation message
    virtual void onClick(InputMouseButtonName button)
    {
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        {
            sceneRef.showFactionMenu(true);
        }
    }
};


class OnClickDeploymentMenu : public OnClickEvent
{
public:
    OnClickDeploymentMenu()
    {}

    virtual void onClick(InputMouseButtonName button)
    {
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        {
            Application::get()->switchScene((Scene*)new DeploymentMenu());
        }
    }
};


class OnClickLogout : public OnClickEvent
{
public:
    OnClickLogout()
    {}

    // TODO: Send the actual logout message
    virtual void onClick(InputMouseButtonName button)
    {
        if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
        {
            Application::get()->switchScene((Scene*)(new LoginMenu));
        }
    }
};


std::unordered_map<std::string, Texture*> MainMenu::s_textures;
std::unordered_map<std::string, FactionBuilding> MainMenu::s_factionBuildings;

MainMenu::MainMenu()
{}

MainMenu::~MainMenu()
{
    for (const std::pair<std::string, Texture*>& t : s_textures)
        delete t.second;

    Client::get_instance()->clearOnMessageEvents();

    delete _mainFuncPanel;
    delete _createFactionPanel;
}

void MainMenu::init()
{
    activeCamera = create_camera({0, 1.0f, 0}, 0.0f, 0.0f);

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
    _mainFuncPanel = new Panel(
        { ConstraintType::PIXEL_CENTER_HORIZONTAL, -250},
        { ConstraintType::PIXEL_CENTER_VERTICAL, 180},
        { 130, 24 }
    );
    _mainFuncPanel->addButton("Enter Planet", new OnClickDeploymentMenu);
    _mainFuncPanel->addButton("Options", nullptr);
    _mainFuncPanel->addButton("Logout", new OnClickLogout);

    _mainFuncPanel->setActive(false);

    // Create faction panel (displayed on login only if no user faction data was found)
    _createFactionPanel = new Panel(
        { ConstraintType::PIXEL_CENTER_HORIZONTAL, 0},
        { ConstraintType::PIXEL_CENTER_VERTICAL, 70},
        { 130, 24 },
        true, { 300, 140 }
    );
    _createFactionPanel->addText("");
    _createFactionPanel->addText("Name your faction");
    _createFactionPanel->addInputField("", 300, nullptr);
    _createFactionPanel->addButton("Create", new OnClickCreateFaction(*this));

    uint32_t factionTxtEntity = create_text(
        "TestFactionNameHere123", 
        ConstraintType::PIXEL_CENTER_HORIZONTAL, 120,
        ConstraintType::PIXEL_CENTER_VERTICAL, 180
    ).first;

    uint32_t factionHQButton = create_button(
        "HQ", 
        ConstraintType::PIXEL_CENTER_HORIZONTAL, -110,
        ConstraintType::PIXEL_CENTER_VERTICAL, 50,
        45, 30,
        nullptr,
        true
    );
    uint32_t factionLaunchPadButton = create_button(
        "Launch Pad", 
        ConstraintType::PIXEL_CENTER_HORIZONTAL, 0,
        ConstraintType::PIXEL_CENTER_VERTICAL, 120,
        130, 30,
        nullptr,
        true
    );
    uint32_t factionBarracksButton = create_button(
        "Barracks", 
        ConstraintType::PIXEL_CENTER_HORIZONTAL, 100,
        ConstraintType::PIXEL_CENTER_VERTICAL, 50,
        100, 30,
        nullptr,
        true
    );
    uint32_t factionBuildingsEntity = createEntity();
    Scene* asRawScene = (Scene*)this;
    asRawScene->addChild(factionBuildingsEntity, factionTxtEntity);
    asRawScene->addChild(factionBuildingsEntity, factionHQButton);
    asRawScene->addChild(factionBuildingsEntity, factionLaunchPadButton);
    asRawScene->addChild(factionBuildingsEntity, factionBarracksButton);
    _buildingsUIComponents = asRawScene->getAllComponents(factionBuildingsEntity);
    // Hide all initially in case user doesn't have a faction yet
    for (Component* c : _buildingsUIComponents)
        c->setActive(false);

    // Create textures
    TextureSampler buildingTextureSampler =
    {
        TextureSamplerFilterMode::PK_SAMPLER_FILTER_MODE_NEAR,
        TextureSamplerAddressMode::PK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        2
    };
    s_textures["FactionBuildings"] = new WebTexture("assets/FactionBuildings.png", buildingTextureSampler, 4);

    // Create all FactionBuildings
    // TODO: Get user's faction buildings from server!
    FactionBuilding launchPadBuilding(
        "LaunchPad", 
        { 0.0f, 0.0f, -8.0f}, { 2.0f, 2.0f},
        (Scene*)this,
        s_textures["FactionBuildings"],
        { 0.0f, 0.0f }
    );
    FactionBuilding spaceHQBuilding(
        "SpaceHQ", 
        { -1.0f, 0.0f, -6.0f}, { 1.2f, 1.2f},
        (Scene*)this,
        s_textures["FactionBuildings"],
        { 0.0f, 1.0f }
    );
    FactionBuilding barracksBuilding(
        "Barracks", 
        { 1.0f, 0.0f, -6.0f}, { 0.8f, 0.8f},
        (Scene*)this,
        s_textures["FactionBuildings"],
        { 0.0f, 2.0f }
    );
    s_factionBuildings.insert(std::make_pair("LaunchPad", launchPadBuilding));
    s_factionBuildings.insert(std::make_pair("SpaceHQ", spaceHQBuilding));
    s_factionBuildings.insert(std::make_pair("Barracks", barracksBuilding));

    Client* client = Client::get_instance();
    client->addOnMessageEvent(
        MESSAGE_TYPE__GetServerMessage, 
        (OnMessageEvent*)(new OnMessageMOTD(_pMOTDTxt))
    );
    client->send(MESSAGE_TYPE__GetServerMessage, {});
}

void MainMenu::update()
{}

void MainMenu::showFactionMenu(bool arg)
{
    _createFactionPanel->setActive(false);
    _mainFuncPanel->setActive(true);
    for (Component* c : _buildingsUIComponents)
        c->setActive(true);
}
