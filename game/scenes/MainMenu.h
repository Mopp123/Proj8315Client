#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "BaseScene.h"
#include "../net/Client.h"
#include "../ui/ui.h"
#include <unordered_map>


class MainMenu : public BaseScene
{
private:
    // On click events
    class OnClickCreateFaction : public pk::ui::OnClickEvent
    {
    public:
        pk::TextRenderable* pCreateFactionInputFieldTxt = nullptr;

        OnClickCreateFaction(pk::TextRenderable* pCreateFactionInputFieldTxt)
        {
            this->pCreateFactionInputFieldTxt = pCreateFactionInputFieldTxt;
        }

        virtual void onClick(pk::InputMouseButtonName button);
    };


    class OnClickDeploymentMenu : public pk::ui::OnClickEvent
    {
    public:
        virtual void onClick(pk::InputMouseButtonName button);
    };

    class OnClickLogout : public pk::ui::OnClickEvent
    {
    public:
        // TODO: Send the actual logout message
        virtual void onClick(pk::InputMouseButtonName button);
    };

    // On message events
    class OnMessageMOTD : public net::OnMessageEvent
    {
    public:
        pk::TextRenderable* pMOTDTxt = nullptr;

        OnMessageMOTD(pk::TextRenderable* pMOTD) :
            pMOTDTxt(pMOTD)
        {}

        virtual void onMessage(const PK_byte* data, size_t dataSize);
    };

    class OnMessageCreateFactionResponse : public net::OnMessageEvent
    {
    public:
        MainMenu& sceneRef;

        OnMessageCreateFactionResponse(MainMenu& sceneRef) :
            sceneRef(sceneRef)
        {}

        virtual void onMessage(const PK_byte* data, size_t dataSize);
    };

    Panel _mainFuncPanel;
    Panel _createFactionPanel;

    pk::TextRenderable* _pMOTDTxt = nullptr;
    pk::TextRenderable* _pFactionInfoTxt = nullptr;

public:
    MainMenu();
    ~MainMenu();

    virtual void init();
    virtual void update();

    void showFactionMenu(bool arg, const std::string& factionName);
};
