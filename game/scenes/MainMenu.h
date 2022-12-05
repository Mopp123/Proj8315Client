#pragma once

#include "../../PortablePesukarhu/ppk.h"
#include "../ui/ui.h"

class MainMenu : public pk::Scene
{
private:
    Panel* _mainFuncPanel = nullptr;

public:
    MainMenu();
    ~MainMenu();

    virtual void init();
    virtual void update();
};
