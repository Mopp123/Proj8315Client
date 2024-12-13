
#pragma once

#include "Tile.h"
#include "world/World.h"
#include "CameraUtils.h"
#include "pesukarhu/ppk.h"


class TravelMenu : public pk::ui::TopBarPanel
{
private:
    class OnClickTravel : public pk::ui::GUIButton::OnClickEvent
    {
    public:
        TravelMenu* pMenu;
        OnClickTravel(TravelMenu* pMenu) : pMenu(pMenu) {}
        void onClick(pk::InputMouseButtonName button);
    };

    pk::ui::InputField* _pInputFieldX;
    pk::ui::InputField* _pInputFieldZ;

    world::World* _pWorld = nullptr;
    CameraController* _pCamController = nullptr;

public:
    void init(
        pk::Scene* pScene,
        world::World* pWorld,
        CameraController* pCamController,
        pk::Font* pFont
    );

    virtual void open();
    virtual void close();

    void setTargetCoords(int32_t targetX, int32_t targetZ);
    void travel();
};
