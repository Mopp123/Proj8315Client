#include "SpawnMenu.h"
#include "../../Proj8315Common/src/Tile.h"
#include "Object.h"
#include "world/Objects.h"
#include "net/Client.h"


using namespace pk;
using namespace pk::ui;
using namespace world;
using namespace world::objects;
using namespace gamecommon;
using namespace net;


void SpawnMenu::OnClickClose::onClick(pk::InputMouseButtonName button)
{
    _pMenu->close();
}


void SpawnMenu::MenuItemOnClick::onClick(pk::InputMouseButtonName button)
{
    Debug::log("___TEST___spawning object: " + std::to_string(_itemIndex));
}


void SpawnMenu::init(pk::Scene* pScene, pk::Font* pFont)
{
    vec2 scale(208, 300);
    initBase(
        pScene,
        pFont,
        "Admin Spawn menu",
        { HorizontalConstraintType::PIXEL_CENTER_HORIZONTAL, -scale.x * 0.5f },
        { VerticalConstraintType::PIXEL_CENTER_VERTICAL, scale.y * 0.5f },
        scale,
        new OnClickClose(this)
    );
    _layoutType = LayoutFillType::VERTICAL;
    for (int i = 0; i < _maxSpawnButtons; ++i)
    {
        // NOTE: we always ignore spawnable object at index 0 which is the "empty" object!
        // -> thats why giving indices to the OnClick events as +1
        _spawnSelectionButtonEntities.push_back(
            addDefaultButton("None", new MenuItemOnClick(this, i + 1), 200)
        );
    }
    close();
}

void SpawnMenu::open(uint64_t tileData)
{
    setComponentsActive(true);

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

void SpawnMenu::displaySpawnButtons(const std::vector<gamecommon::ObjectInfo>& objects)
{
    // This shit might fuck up stuff in the future...
    TopBarPanel::setComponentsActive(true);

    if (objects.size() >= _spawnSelectionButtonEntities.size())
    {
        Debug::log(
            "@SpawnMenu::displaySpawnButtons "
            "Not enought available buttons to display objects"
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
