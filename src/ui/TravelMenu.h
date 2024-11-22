
#pragma once

#include "Tile.h"
#include "TopBarPanel.h"
#include "ecs/factories/ui/UIFactories.h"

class TravelMenu : public TopBarPanel
{
private:
    class OnClickTravel : public pk::ui::OnClickEvent
    {
    public:
        TravelMenu* pMenu;
        OnClickTravel(TravelMenu* pMenu) : pMenu(pMenu) {}
        void onClick(pk::InputMouseButtonName button);
    };

    entityID_t _inputFieldEntityX = 0;
    entityID_t _inputFieldEntityZ = 0;



public:
    void init(pk::Scene* pScene, pk::Font* pFont);

    virtual void open();
    virtual void close();

    void setTargetCoords(int32_t targetX, int32_t targetZ);
    void travel();

private:
    void setInputFieldContents(const std::string& targetX, const std::string& targetZ);
    void getInputFieldContents(std::string& outTargetX, std::string& outTargetZ) const;
};
