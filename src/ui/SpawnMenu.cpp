#include "SpawnMenu.h"
#include "../../Proj8315Common/src/Tile.h"
#include "../../Proj8315Common/src/messages/AdminMessages.h"
#include "Object.h"
#include "messages/Message.h"
#include "world/Objects.h"
#include "net/Client.h"


using namespace pk;
using namespace pk::ui;
using namespace world;
using namespace world::objects;
using namespace gamecommon;
using namespace net;


void SpawnMenu::MenuItemOnClick::onClick(pk::InputMouseButtonName button)
{
    Client* pClient = Client::get_instance();
    if (!pClient)
    {
        Debug::log(
            "@SpawnMenu::MenuItemOnClick::onClick "
            "Client was nullptr!",
            Debug::MessageType::PK_ERROR
        );
        return;
    }
    const User& user = pClient->user;
    if (!user.isLoggedIn())
    {
        Debug::log(
            "@SpawnMenu::MenuItemOnClick::onClick "
            "User isn't logged in!",
            Debug::MessageType::PK_ERROR
        );
        return;
    }

    // Assuming here that it is already checked that the user is admin...
    // (The following messages doesn't go through anyways if user isn't admin type)
    pClient->send(
        (int32_t)MESSAGE_TYPE__SpawnRequest,
        {
            {
                (GC_byte*)&_itemIndex,
                TILE_STATE_SIZE_thingID,
                TILE_STATE_SIZE_thingID
            },
            {
                (GC_byte*)&_pMenu->_selectedTileX,
                sizeof(int32_t),
                sizeof(int32_t)
            },
            {
                (GC_byte*)&_pMenu->_selectedTileY,
                sizeof(int32_t),
                sizeof(int32_t)
            }
        }
    );
    _pMenu->close();
}


void SpawnMenu::init(pk::Scene* pScene, pk::Font* pFont)
{
    vec2 scale(208, 155);
    initBase(
        pScene,
        pFont,
        "Admin Spawn menu",
        {
            HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL,
            -scale.x * 0.5f,
            VerticalConstraintType::PIXEL_CENTER_VERTICAL,
            scale.y * 0.5f
        },
        scale
    );
    _layoutType = LayoutFillType::VERTICAL;

    if (ObjectInfoLib::getObjectInfos().size() == 0)
    {
        Debug::log(
            "@SpawnMenu::init "
            "Object info lib was empty! "
            "Make sure you init UI after Object info lib has been created!",
            Debug::MessageType::PK_FATAL_ERROR
        );
        return;
    }
    // -1 since empty object
    _maxSpawnButtons = ObjectInfoLib::getObjectInfos().size() - 1;
    for (int i = 0; i < _maxSpawnButtons; ++i)
    {
        // NOTE: we always ignore spawnable object at index 0 which is the "empty" object!
        // -> thats why giving indices to the OnClick events as +1
        _spawnSelectionButtonEntities.push_back(
            addDefaultButton("None", new MenuItemOnClick(this, i + 1), 200)
        );
    }
    close();

    setLayer(1);
}

void SpawnMenu::open()
{
    //setComponentsActive(true);
    std::vector<Component*> components = _pScene->getAllComponents(_entity);

    std::vector<ObjectInfo>& spawnableObjects = ObjectInfoLib::getObjectInfos();
    if (spawnableObjects.empty())
    {
        Debug::log(
            "@SpawnMenu::open No objects to spawn. ObjectInfoLib was empty!"
        );
        return;
    }

    displaySpawnButtons(spawnableObjects);
}

void SpawnMenu::close()
{
    setComponentsActive(false);
    for (UIFactoryButton& b : _spawnSelectionButtonEntities)
    {
        std::vector<Component*> components = _pScene->getComponents(b.rootEntity);
        std::vector<Component*> imgComponents = _pScene->getComponents(b.imgEntity);
        std::vector<Component*> txtComponents = _pScene->getComponents(b.txtEntity);
        components.insert(components.end(), imgComponents.begin(), imgComponents.end());
        components.insert(components.end(), txtComponents.begin(), txtComponents.end());
        for (Component* pComponent : components)
            pComponent->setActive(false);
    }
}

void SpawnMenu::setSelectedTile(uint64_t tileData, int32_t x, int32_t y)
{
    _selectedTileData = tileData;
    _selectedTileX = x;
    _selectedTileY = y;
}

void SpawnMenu::displaySpawnButtons(const std::vector<gamecommon::ObjectInfo>& objects)
{
    // This shit might fuck up stuff in the future...
    TopBarPanel::setComponentsActive(true);

    // -1 since objects contains the "empty" object as well
    if (objects.size() - 1 > _spawnSelectionButtonEntities.size())
    {
        Debug::log(
            "@SpawnMenu::displaySpawnButtons "
            "Not enought available buttons to display objects. "
            "Obj info lib length: " + std::to_string(objects.size()) + " "
            "Available buttons: " + std::to_string(_spawnSelectionButtonEntities.size())
        );
        return;
    }

    // Starting from index 1 since 0 is a "mandatory empty object" in the obj info lib
    for (int i = 1; i < objects.size(); ++i)
    {
        const ObjectInfo& objInfo = objects[i];
        std::string objName(objInfo.name);
        UIFactoryButton& button = _spawnSelectionButtonEntities[i - 1];

        std::vector<Component*> components = _pScene->getComponents(button.rootEntity);
        std::vector<Component*> imgComponents = _pScene->getComponents(button.imgEntity);
        std::vector<Component*> txtComponents = _pScene->getComponents(button.txtEntity);
        components.insert(components.end(), imgComponents.begin(), imgComponents.end());
        components.insert(components.end(), txtComponents.begin(), txtComponents.end());
        for (Component* pComponent : components)
            pComponent->setActive(true);

        TextRenderable* pTxtRenderable = (TextRenderable*)_pScene->getComponent(
            button.txtEntity,
            ComponentType::PK_RENDERABLE_TEXT
        );
        pTxtRenderable->accessStr() = objName;
    }
}
