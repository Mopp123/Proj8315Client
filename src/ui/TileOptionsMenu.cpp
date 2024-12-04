#include "TileOptionsMenu.h"
#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/Tile.h"
#include "Object.h"
#include "world/Objects.h"
#include "net/Client.h"


using namespace pk;
using namespace pk::ui;
using namespace world;
using namespace gamecommon;
using namespace net;


static void menu_item_func_spawn(TileOptionsMenu* pMenu)
{
    pMenu->accessSpawnMenu().open();
    pMenu->accessTerrainToolMenu().close();
    pMenu->accessTravelMenu().close();
    pMenu->close();
}


static void menu_item_func_terrain(TileOptionsMenu* pMenu)
{
    pMenu->accessSpawnMenu().close();
    pMenu->accessTerrainToolMenu().open();
    pMenu->accessTravelMenu().close();
    pMenu->close();
}


static void menu_item_func_travel(TileOptionsMenu* pMenu)
{
    pMenu->accessSpawnMenu().close();
    pMenu->accessTerrainToolMenu().close();
    pMenu->accessTravelMenu().open();
    pMenu->close();
    pMenu->close();
}


static void menu_item_func_cancel(TileOptionsMenu* pMenu)
{
    pMenu->close();
}


void TileOptionsMenu::MenuMouseButtonEvent::func(
    InputMouseButtonName button,
    InputAction action,
    int mods
)
{
    if (!pMenu->isMouseOver())
    {
        if (action != InputAction::PK_INPUT_RELEASE)
            pMenu->close();
    }
}


void TileOptionsMenu::MenuItemOnClick::onClick(pk::InputMouseButtonName button)
{
    if (_itemIndex >= _pMenu->_activeItems.size())
    {
        Debug::log(
            "@TileOptionsMenu::MenuItemOnClick::onClick "
            "_itemIndex: " + std::to_string(_itemIndex) + " out of bounds! "
            "Currently active item count: " + std::to_string(_pMenu->_activeItems.size()),
            Debug::MessageType::PK_FATAL_ERROR
        );
        return;
    }
    void (*func)(TileOptionsMenu*) = _pMenu->_activeItems[_itemIndex].func;
    if (!func)
    {
        Debug::log(
            "@TileOptionsMenu::MenuItemOnClick::onClick "
            "Item: " + std::to_string(_itemIndex) + " function was nullptr!",
            Debug::MessageType::PK_FATAL_ERROR
        );
        return;
    }
    func(_pMenu);
}


void TileOptionsMenu::init(
    pk::Scene* pScene,
    world::World* pWorld,
    CameraController* pCamController,
    pk::Font* pFont
)
{
    const vec2 panelScale(134, 160);
    const vec2 slotScale(130, 20);
    createDefault(
        pScene,
        pFont,
        {
            HorizontalConstraintType::PIXEL_LEFT, 0.0f,
            VerticalConstraintType::PIXEL_BOTTOM, 60.0f,
        },
        panelScale,
        slotScale,
        Panel::LayoutFillType::VERTICAL,
        0
    );

    _buttonTxtDisplacementX = _slotScale.x * 0.125f;

    for (int i = 0; i < _maxMenuItems; ++i)
    {
        _menuButtons.push_back(
            addDefaultButton("Item" + std::to_string(i), new MenuItemOnClick(this, i), slotScale.x)
        );
    }

    Application::get()->accessInputManager()->addMouseButtonEvent(new MenuMouseButtonEvent(this));

    close();

    // Create admin menus which can be opened from here
    _spawnMenu.init(pScene, pFont);
    _terrainToolMenu.init(pScene, pFont);
    _travelMenu.init(pScene, pWorld, pCamController, pFont);

    setLayer(2);
}

void TileOptionsMenu::open(float screenX, float screenY, uint64_t tileData, int tileX, int tileY)
{
    // Make sure prev items are cleared
    close();
    updateActiveItemsList(tileData);
    if (_activeItems.empty())
    {
        Debug::log(
            "@TileOptionsMenu::open "
            "No menu items available",
            Debug::MessageType::PK_WARNING
        );
        return;
    }

    _spawnMenu.setSelectedTile(tileData, tileX, tileY);
    _terrainToolMenu.setSelectedTile(tileData, tileX, tileY);
    _travelMenu.setTargetCoords(tileX, tileY);

    std::vector<Component*> rootComponents = _pScene->getComponents(_entity);
    for (Component* pComponent: rootComponents)
        pComponent->setActive(true);

    // Alter the panel's height depending on how many items to display
    Transform* pTransform = (Transform*)_pScene->getComponent(
        _entity,
        ComponentType::PK_TRANSFORM
    );
    mat4& tMat = pTransform->accessTransformationMatrix();

    const float& menuWidth = tMat[0 + 0 * 4];
    float menuHeight = _activeItems.size() * (_slotScale.y + _slotPadding);

    tMat[1 + 1 * 4] = menuHeight;

    // Push the menu somewhere else if goes out of bounds of the current window
    const Window* pWindow = Application::get()->getWindow();
    const Swapchain* pSwapchain = pWindow->getSwapchain();
    Extent2D viewportExtent = pSwapchain->getSurfaceExtent();

    float useX = screenX;
    float useY = screenY;

    if (screenX + menuWidth > (float)viewportExtent.width)
        useX = viewportExtent.width - menuWidth;
    if (screenY - menuHeight < 0)
        useY = menuHeight;

    ConstraintData* pConstraintData = (ConstraintData*)_pScene->getComponent(
        _entity,
        ComponentType::PK_UI_CONSTRAINT
    );
    pConstraintData->horizontalValue = useX;
    pConstraintData->verticalValue = useY - menuHeight;

    for (int i = 0; i < _activeItems.size(); ++i)
        displayButton(useX, useY, i, _activeItems[i].txt);
}

void TileOptionsMenu::close()
{
    _activeItems.clear();

    std::vector<Component*> components = _pScene->getComponents(_entity);
    for (Component* pComponent : components)
        pComponent->setActive(false);

    for (UIFactoryButton b : _menuButtons)
        setButtonActive(b, false);
}

void TileOptionsMenu::displayButton(float x, float y, int index, const std::string& txt)
{
    if (index >= _maxMenuItems)
    {
        Debug::log(
            "@TileOptionsMenu::displayButton "
            "button index: " + std::to_string(index) + " out of bounds. "
            "max button count is " + std::to_string(_maxMenuItems),
            Debug::MessageType::PK_FATAL_ERROR
        );
        return;
    }

    UIFactoryButton menuButton = _menuButtons[index];
    setButtonActive(menuButton, true);
    entityID_t buttonImg = menuButton.imgEntity;
    entityID_t buttonTxt = menuButton.txtEntity;

    ConstraintData* pImgConstraint = (ConstraintData*)_pScene->getComponent(
        buttonImg,
        ComponentType::PK_UI_CONSTRAINT
    );
    ConstraintData* pTxtConstraint = (ConstraintData*)_pScene->getComponent(
        buttonTxt,
        ComponentType::PK_UI_CONSTRAINT
    );

    float yPos = y - _slotScale.y - _slotPadding * 0.5f - (index * (_slotScale.y + _slotPadding));
    pImgConstraint->horizontalValue = x + _slotPadding * 0.5f;
    pImgConstraint->verticalValue = yPos;

    // Add quite a lot of text padding so when opening menu cursor doesn't block view to the first element..
    // ...like in most of these kinds of "right click menus"
    pTxtConstraint->horizontalValue = x + _buttonTxtDisplacementX;
    pTxtConstraint->verticalValue = yPos;

    TextRenderable* pTextRenderable = (TextRenderable*)_pScene->getComponent(
        buttonTxt,
        ComponentType::PK_RENDERABLE_TEXT
    );
    pTextRenderable->accessStr() = txt;
}

void TileOptionsMenu::setButtonActive(UIFactoryButton& button, bool arg)
{
    // Fucking awful way of getting all the components atm I know...
    std::vector<Component*> buttonComponents = _pScene->getComponents(button.rootEntity);
    std::vector<Component*> buttonImgComponents = _pScene->getComponents(button.imgEntity);
    std::vector<Component*> buttonTxtComponents = _pScene->getComponents(button.txtEntity);
    for (Component* pComponent : buttonComponents)
        pComponent->setActive(arg);
    for (Component* pComponent : buttonImgComponents)
        pComponent->setActive(arg);
    for (Component* pComponent : buttonTxtComponents)
        pComponent->setActive(arg);
}

// Fucking disgusting but require some way to either open menu or do what the fuck we ever want
// with any menu item...
void TileOptionsMenu::updateActiveItemsList(uint64_t tile)
{
    if (!_activeItems.empty())
    {
        Debug::log(
            "@TileOptionsMenu::updateActiveItemsList "
            "Active items list was already populated! "
            "(item count: " + std::to_string(_activeItems.size()) + ")",
            Debug::MessageType::PK_FATAL_ERROR
        );
        return;
    }

    GC_ubyte elevation = get_tile_terrelevation(tile);
    GC_ubyte tileType = get_tile_terrtype(tile);
    GC_ubyte objectType = get_tile_thingid(tile);

    Client* pClient = Client::get_instance();
    if (!pClient->isConnected())
    {
        Debug::log(
            "@TileOptionsMenu::updateActiveItemsList "
            "Client wasn't connected",
            Debug::MessageType::PK_FATAL_ERROR
        );
        return;
    }
    bool isAdmin = true;
    User& user = pClient->user;
    if (!user.isLoggedIn())
    {
        Debug::log(
            "@TileOptionsMenu::updateActiveItemsList "
            "User wasn't logged in!",
            Debug::MessageType::PK_FATAL_ERROR
        );
        return;
    }
    isAdmin = user.isAdmin();

    _activeItems.reserve(_maxMenuItems);
    DropDownMenuItem cancel = {
        "Cancel",
        menu_item_func_cancel
    };
    if (isAdmin)
    {
        DropDownMenuItem spawn = {
            "Spawn menu",
            menu_item_func_spawn
        };
        DropDownMenuItem alterTerrain = {
            "Alter terrain",
            menu_item_func_terrain
        };
        _activeItems.emplace_back(spawn);
        _activeItems.emplace_back(alterTerrain);
    }

    DropDownMenuItem travel = {
        "Travel",
        menu_item_func_travel
    };
    _activeItems.emplace_back(travel);
    _activeItems.emplace_back(cancel);
}
