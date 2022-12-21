#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "../ui/ui.h"
#include <unordered_map>


class FactionBuilding
{
private:
    uint32_t _entity = 0;
    pk::Sprite3DRenderable* _pRenderable = nullptr;
    std::string _type;

public:
    FactionBuilding(
        std::string type, 
        pk::vec3 pos, 
        pk::vec2 scale, 
        pk::Scene* scene, 
        pk::Texture* spriteSheetTexture,
        pk::vec2 texOffset
    );

    FactionBuilding(const FactionBuilding& other);
};


class MainMenu : public pk::Scene
{
private:
    Panel* _mainFuncPanel = nullptr;
    Panel* _createFactionPanel = nullptr;
    std::vector<pk::Component*> _buildingsUIComponents;

    pk::web::WebTexture* _pBuildingsTexture = nullptr;
    bool _userFactionExists = false;

    pk::TextRenderable* _pMOTDTxt = nullptr;

    static std::unordered_map<std::string, pk::Texture*> s_textures;
    static std::unordered_map<std::string, FactionBuilding> s_factionBuildings;

public:
    MainMenu();
    ~MainMenu();

    virtual void init();
    virtual void update();

    void showFactionMenu(bool arg);
};
